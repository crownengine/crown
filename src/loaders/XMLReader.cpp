/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdio>
#include <ctype.h>
#include "Auto.h"
#include "Log.h"
#include "Types.h"
#include "Str.h"
#include "XMLReader.h"
#include "Stream.h"
#include "MathUtils.h"
#include "Filesystem.h"

namespace Crown
{

enum eXMLTokenizerState
{
	XTS_WHITESPACE,
	XTS_WORD,
	XTS_STRING,
	XTS_ESCAPE,
	XTS_SLASH,
	XTS_LESSTHAN
};

XMLReader::XMLReader():
  mRoot(NULL), mReadToken(XTT_UNDEFINED)
{
}

XMLReader::~XMLReader()
{
	Clear();
}

void XMLReader::Print()
{
	if (mRoot)
		PrintNode(mRoot, 0);
	else
		Log::I("XMLReader::Print: Empty XML or Error in loading");
}

void XMLReader::PrintNode(XMLNode* node, int depth)
{
	Str tag = "<" + node->name;

	Dictionary<Str, Str>::Enumerator e = node->simpleAttributes.getBegin();
	while (e.next())
	{
		tag += " " + e.current().key + "=\"" + e.current().value + "\"";
	}

	if (node->simpleAttributes.GetSize() == 0)
		tag += " />";
	else
		tag += " >";

	Log::I(Str(depth, ' ') + tag);

	for(int i=0; i<node->children.GetSize(); i+=1)
	{
		PrintNode(node->children[i], depth + 1);
	}

	Log::I(Str(depth, ' ') + "</" + node->name + ">");
}

bool XMLReader::LoadFile(Str filePath)
{
	//This local Shared pointer keeps the stream alive, recursive calls can use plain pointers safely
	Stream* streamIn = GetFilesystem()->OpenStream(filePath, SOM_READ);
	if (streamIn == NULL)
		return false;

	if (!InitTokenizer(streamIn))
		return false;

	bool dummy;
	mRoot = ReadNode(streamIn, NULL, dummy);

	if (mRoot == NULL)
		return false;

	GetFilesystem()->Close(streamIn);
	return true;
}

XMLNode* XMLReader::ReadNode(Stream* streamIn, XMLNode* parentNode, bool& isComplexProperty)
{
	Auto<XMLNode> node = new XMLNode();

	bool accepted;
	if (!Accepted(streamIn, XTT_TAG_OPEN, accepted))
		return NULL;

	if (!accepted)
		return NULL;

	if (!Expected(streamIn, XTT_WORD))
		return NULL;

	node->name = mReadValue;

	isComplexProperty = false;
	bool dotFound = false;
	if (!Accepted(streamIn, XTT_DOT, dotFound))
		return NULL;
	
	if (dotFound)
	{
		if (parentNode == NULL)
		{
			Log::E("A complex property can only be specified inside a tag");
			return NULL;
		}

		if (parentNode->name != node->name)
		{
			Log::E("The name before the dot in a complex property must be the same as the parent tag. Expected '" + parentNode->name + "' but '" + node->name + "' found.");
			return NULL;
		}

		//Case of complex property
		//eg: Pear.Leaves in <Pear><Pear.Leaves>...</Pear.Leaves></Pear>
		//Expect a dot and another name
		isComplexProperty = true;

		if (!Expected(streamIn, XTT_WORD))
			return NULL;

		node->name = mReadValue;
	}

	bool isInlineTag;

	while (true)
	{
		bool accepted;
		//Accept a > and proceed with children parsing
		if (!Accepted(streamIn, XTT_TAG_CLOSE, accepted))
			return NULL;

		if (accepted)
		{
			isInlineTag = false;
			break;
		}

		//Accept a /> and avoid parsing children
		if (!Accepted(streamIn, XTT_TAG_INLINECLOSE, accepted))
			return NULL;

		if (accepted)
		{
			isInlineTag = true;
			break;
		}

		if (!Expected(streamIn, XTT_WORD))
			return NULL;
		Str attributeName = mReadValue;

		if (!Expected(streamIn, XTT_EQUAL))
			return NULL;

		if (!Expected(streamIn, XTT_STRING))
			return NULL;
		node->simpleAttributes[attributeName] = mReadValue;
	}

	if (!isInlineTag)
	{

		while(true)
		{
			bool isChildComplexProperty;
			XMLNode* child = ReadNode(streamIn, node.GetPointer(), isChildComplexProperty);
			if (child == NULL)
				break;

			if (isChildComplexProperty)
			{
				node->complexAttributes[child->name] = child;
			}
			else
			{
				node->children.Append(child);
			}
		}

		if (!Expected(streamIn, XTT_TAG_OPENEXPLICIT))
			return NULL;

		if (!Expected(streamIn, XTT_WORD))
			return NULL;

		if (isComplexProperty)
		{
			if (mReadValue != parentNode->name)
			{
				Log::E("Closing tag of '" + parentNode->name + "." + node->name + "' is mismatched");
				return NULL;
			}

			if (!Expected(streamIn, XTT_DOT))
				return NULL;

			if (!Expected(streamIn, XTT_WORD))
				return NULL;
		}

		if (mReadValue != node->name)
		{
			Log::E("Closing tag of '" + node->name + "' is mismatched");
			return NULL;
		}

		if (!Expected(streamIn, XTT_TAG_CLOSE))
			return NULL;
	}

	return node.GetPointer(true);
}

bool XMLReader::InitTokenizer(Stream* streamIn)
{
	mStreamEnded = streamIn->EndOfStream();
  if (mStreamEnded)
		return false;
	mCurrentCharacter = streamIn->ReadByte();

	return true;
}

bool XMLReader::NextTokenIfUndefined(Stream* streamIn)
{
	if (mStreamEnded)
		return false;
	if (mReadToken != XTT_UNDEFINED)
		return true;

	mReadToken = XTT_UNDEFINED;
	eXMLTokenizerState state = XTS_WHITESPACE;

	while (!mStreamEnded)
	{
		bool exitWhile = false;

		switch (state)
		{
			case XTS_WHITESPACE:
			{
				if (isalpha(mCurrentCharacter))
				{
					state = XTS_WORD;
					mReadToken = XTT_WORD;
					mReadValue = Str(mCurrentCharacter);
				}
				else if (mCurrentCharacter == '"')
				{
					state = XTS_STRING;
					mReadToken = XTT_STRING;
					mReadValue = Str::Empty();
				}
				else if (mCurrentCharacter == '<')
				{
					state = XTS_LESSTHAN;
					mReadToken = XTT_TAG_OPEN;
				}
				else if (mCurrentCharacter == '>')
				{
					mReadToken = XTT_TAG_CLOSE;
					exitWhile = true;
				}
				else if (mCurrentCharacter == '/')
				{
					state = XTS_SLASH;
				}
				else if (mCurrentCharacter == '=')
				{
					mReadToken = XTT_EQUAL;
					exitWhile = true;
				}
				else if (mCurrentCharacter == '.')
				{
					mReadToken = XTT_DOT;
					exitWhile = true;
				}
				else if (!isspace(mCurrentCharacter) && mCurrentCharacter != '\n' && mCurrentCharacter != '\r')
					return false;
			}
			break;
			case XTS_WORD:
			{
				if (isalpha(mCurrentCharacter))
				{
					mReadValue += Str(mCurrentCharacter);
				}
				else
					return true;
			}
			break;
			case XTS_STRING:
			{
				if (mCurrentCharacter == '\\')
					state = XTS_ESCAPE;
				else if (mCurrentCharacter == '"')
					exitWhile = true;
				else
					mReadValue += Str(mCurrentCharacter);
			}
			break;
			case XTS_ESCAPE:
			{
				if (mCurrentCharacter == '\\' || mCurrentCharacter == '"')
				{
					state = XTS_STRING;
					mReadValue += Str(mCurrentCharacter);
				}
				else
					return false;
			}
			break;
			case XTS_SLASH:
			{
				if (mCurrentCharacter == '>')
				{
					mReadToken = XTT_TAG_INLINECLOSE;
					exitWhile = true;
				}
				else
					return false;
			}
			break;
			case XTS_LESSTHAN:
			{
				if (mCurrentCharacter == '/')
				{
					mReadToken = XTT_TAG_OPENEXPLICIT;
					exitWhile = true;
				}
				else
					return true;
			}
			break;
		}

		if (streamIn->EndOfStream())
			mStreamEnded = true;
		else
			mCurrentCharacter = streamIn->ReadByte();

		if (exitWhile)
			break;
	}

	if (state == XTS_ESCAPE || (state == XTS_SLASH && mReadToken != XTT_TAG_INLINECLOSE))
		return false;

	return true;
}

eXMLTokenType XMLReader::ConsumeCurrentToken()
{
	eXMLTokenType ret = mReadToken;
	mReadToken = XTT_UNDEFINED;
	return ret;
}

eXMLTokenType XMLReader::GetCurrentToken()
{
	return mReadToken;
}

bool XMLReader::Expected(Stream* streamIn, eXMLTokenType expectedToken)
{
	if (!NextTokenIfUndefined(streamIn))
		return false;

	if (ConsumeCurrentToken() != expectedToken)
	{
		char buff[21];
		int bytesToRead = Math::Min<int>(20, streamIn->GetSize() - streamIn->GetPosition());
		streamIn->ReadDataBlock(buff, bytesToRead);
		buff[bytesToRead] = '\0';
		Log::E("Unexpected character before '" + Str(buff) + "'");
		mStreamEnded = true;
		return false;
	}

	return true;
}

bool XMLReader::Accepted(Stream* streamIn, eXMLTokenType acceptedToken, bool& accepted)
{
	if (!NextTokenIfUndefined(streamIn))
		return false;

	accepted = (GetCurrentToken() == acceptedToken);

	if (accepted)
		ConsumeCurrentToken();

	return true;
}


bool XMLReader::ExpectedWhitespace(Stream* streamIn)
{
	char currentChar = streamIn->ReadByte();
	if (!isspace(currentChar))
		return false;

	return true;
}

Str XMLReader::ReadWord(Stream* streamIn)
{
	Str out = Str::Empty();
	bool skippingWhite = true;

	while (!streamIn->EndOfStream())
	{
		char currentChar = streamIn->ReadByte();

		if (skippingWhite)
		{
			if (isspace(currentChar))
			{
				continue;
			}
			else
			{
				skippingWhite = false;
			}
		}

		if (!isalpha(currentChar))
		{
			break;
		}

		//TODO: Add a operator+(char n)
		out += Str(currentChar);
	}

	//Undo the last character (if not eof)
	if (!streamIn->EndOfStream())
		streamIn->Seek(-1, SM_SeekFromCurrent);

	return out;
}

void XMLReader::Clear()
{
	if (mRoot)
	{
		delete mRoot;
		mRoot = NULL;
	}
}

} // namespace Crown

