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

#pragma once

#include "Types.h"
#include "Device.h"
#include "Str.h"
#include "Dictionary.h"

namespace Crown
{

class Stream;

enum eXMLTokenType
{
	XTT_UNDEFINED,
	XTT_WORD,
	XTT_TAG_OPEN,
	XTT_TAG_CLOSE,
	XTT_TAG_INLINECLOSE,
	XTT_TAG_OPENEXPLICIT,
	XTT_EQUAL,
	XTT_STRING,
	XTT_DOT
};

class XMLNode
{
public:
	Str name;
	Dictionary<Str, Str> simpleAttributes;		//Attributes whose value is a string
	Dictionary<Str, XMLNode*> complexAttributes;	//Attributes whose value is a XMLNode representing a structured value
	List<XMLNode*> children;

	XMLNode()
	{

	}

	virtual ~XMLNode()
	{
		Dictionary<Str, XMLNode*>::Enumerator e = complexAttributes.getBegin();
		while (e.next())
		{
			delete e.current().value;
		}

		for(int i=0; i<children.GetSize(); i++)
		{
			delete children[i];
		}
	}

	XMLNode* Clone() const
	{
		XMLNode* clone = new XMLNode();
		clone->name = name;

		Dictionary<Str, Str>::Enumerator es = simpleAttributes.getBegin();
		while (es.next())
		{
			clone->simpleAttributes[es.current().key] = es.current().value;
		}

		Dictionary<Str, XMLNode*>::Enumerator ec = complexAttributes.getBegin();
		while (ec.next())
		{
			clone->complexAttributes[ec.current().key] = ec.current().value->Clone();
		}

		for(int i=0; i<children.GetSize(); i++)
		{
			clone->children.Append(children[i]->Clone());
		}

		return clone;
	}
};

class XMLReader
{
public:
	XMLReader();
	~XMLReader();

	bool LoadFile(Str filePath);
	inline XMLNode* GetRootXMLNode()
	  { return mRoot; }
	void Clear();
	void Print();

private:
	XMLNode* mRoot;
	char mCurrentCharacter;
	bool mStreamEnded;
	eXMLTokenType mReadToken;
	Str mReadValue;

	XMLNode* ReadNode(Stream* streamIn, XMLNode* parentNode, bool& isComplexProperty);
	void PrintNode(XMLNode* node, int depth);
	bool InitTokenizer(Stream* streamIn);
	bool NextTokenIfUndefined(Stream* streamIn);
	eXMLTokenType ConsumeCurrentToken();
	eXMLTokenType GetCurrentToken();
	bool Expected(Stream* streamIn, eXMLTokenType expectedToken);
	bool Accepted(Stream* streamIn, eXMLTokenType acceptedToken, bool& accepted);
	bool ExpectedWhitespace(Stream* streamIn);
	Str ReadWord(Stream* streamIn);
};

} // namespace Crown

