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

#include "XWMLReader.h"
#include "Log.h"
#include "Types.h"
#include "Auto.h"
#include "StackLayout.h"
#include "ListView.h"
#include "TextBox.h"
#include "Bind.h"
#include "WindowContext.h"
#include "TreeView.h"
#include "ImageWidget.h"

namespace crown
{

XWMLReader::XWMLReader()
{

}
XWMLReader::~XWMLReader()
{

}

bool XWMLReader::ParseAction(const Str& actionStr, Str& actionName, List<Str>* arguments)
{
	//TODO: Should throw
	if (arguments == NULL)
		return false;

	//The action is in the form {Action actionName(arg0, arg1, ... , argn)}

	if (!actionStr.StartsWith("{Action ") || !actionStr.EndsWith("}"))
		return false;

	Str actionStatement = actionStr.GetSubstring(8, actionStr.GetLength() - 1);

	arguments->Clear();
	int32_t openParenthesisIndex = actionStatement.FindFirst('(');
	if (openParenthesisIndex != -1)
	{
		if (actionStatement[actionStatement.GetLength() - 1] != ')')
			return false;

		actionName = actionStatement.GetSubstring(0, openParenthesisIndex);

		actionStatement = actionStatement.GetSubstring(openParenthesisIndex + 1, actionStatement.GetLength() - 1);
		List<Str> args;
		//TODO: Use something more appropriate than a split to detect commas
		actionStatement.Split(',', args);
		
		for(int32_t i=0; i<args.GetSize(); i++)
		{
			int32_t firstQuoteIndex = args[i].FindFirst('\'');

			if (firstQuoteIndex != -1)
			{
				int32_t lastQuoteIndex = args[i].FindLast('\'');
				//In this case, trimming is not necessary because apices wrap the item well
				arguments->Append(args[i].GetSubstring(firstQuoteIndex + 1, lastQuoteIndex));
			}
			else
			{
				//Do a Trim to remove prefix and postfix spaces
				arguments->Append(args[i].Trim());
			}
		}
	}
	else
	{
		actionName = actionStatement;
	}

	return true;
}

bool XWMLReader::ParseBind(const Str& bindStr, Str& bindPath, Str& source, Str& converter)
{
	//The bind is in the form {Bind bindName} or {Bind bindName,Source=Self}

	if (!bindStr.StartsWith("{Bind ") || !bindStr.EndsWith("}"))
		return false;

	Str values = bindStr.GetSubstring(6, bindStr.GetLength() - 1);

	List<Str> splits;
	values.Split(',', splits);
	
	bindPath = splits[0];
	for(int32_t i = 1; i < splits.GetSize(); i++)
	{
		List<Str> valueSplits;
		splits[i].Split('=', valueSplits);
		if (valueSplits.GetSize() != 2)
		{
			Log::E("Invalid format for Bind parameter '" + splits[i] + "'");
			return false;
		}

		if (valueSplits[0] == "Source")
		{
			source = valueSplits[1];
		}
		else if (valueSplits[0] == "Converter")
		{
			converter = valueSplits[1];
		}
		else
		{
			Log::E("Unknown Bind parameter '" + valueSplits[0] + "'");
			return false;
		}
	}

	return true;
}

Window* XWMLReader::LoadFile(Str filePath, WindowsManager* windowsManager, WindowContext* context)
{
	XMLReader xml;
	if (!xml.LoadFile(filePath))
		return NULL;

	//Verify that the root is a Window element
	XMLNode* rootNode = xml.GetRootXMLNode();

	Auto<Window> window = LoadWindow(rootNode, windowsManager);

	//Initialize the context
	if (context != NULL)
	{
		context->SetAssociatedWindow(window.GetPoint32_ter());
	}

	if (!LoadWidgetPropertiesAndChildren(rootNode, window.GetPoint32_ter()))
		return NULL;

	//window->ApplyBinds();

	if (context != NULL)
	{
		context->OnLoad();
	}

	return window.GetPoint32_ter(true);
}

Window* XWMLReader::LoadWindow(XMLNode* node, WindowsManager* windowsManager)
{
	if (node->name != "Window")
	{
		Log::E("XWMLReader::LoadFile: The root element of a XWML file must be a Window.");
		return NULL;
	}

	Window* window = new Window(windowsManager, 180, 150, "Window");
	return window;
}


Widget* XWMLReader::LoadWidget(XMLNode* node, Widget* parent)
{
	Auto<Widget> widget = CreateWidgetByName(node->name, parent);

	if (widget.IsNull())
	{
		Log::E("XWMLReader::LoadFile: Undefined widget type '" + node->name + "'");
		return NULL;
	}

	if (!LoadWidgetPropertiesAndChildren(node, widget.GetPoint32_ter()))
		return NULL;

	return widget.GetPoint32_ter(true);
}

bool XWMLReader::LoadWidgetPropertiesAndChildren(XMLNode* node, Widget* widget)
{
	Dictionary<Str, Str>::Enumerator es = node->simpleAttributes.getBegin();
	while (es.next())
	{
		Str bindSourcePath, source, converterName;
		if (ParseBind(es.current().value, bindSourcePath, source, converterName))
		{
			Bind* bind;
			Converter* converter = NULL;
			if (converterName != "")
			{
				converter = widget->GetWindow()->GetWindowContext()->GetRegisteredConverter(converterName);
				if (converter == NULL)
				{
					Log::E("Converter '" + converterName + "' is not registered in the Window Context");
				}
			}

			if (source != "")
			{
				if (source == "Self")
				{
					bind = new Bind(widget, widget, bindSourcePath, es.current().key, converter);
				}
				else if (source == "Parent")
				{
					bind = new Bind(widget->GetParent(), widget, bindSourcePath, es.current().key, converter);
				}
				else if (source == "LogicalParent")
				{
					bind = new Bind(widget->GetLogicalParent(), widget, bindSourcePath, es.current().key, converter);
				}
				else if (source == "Widget")
				{
					List<Str> splits;
					bindSourcePath.Split('.', splits);
					if (splits.GetSize() != 2)
					{
						Log::E("Bind Path '" + bindSourcePath + "' is in an invalid format");
					}
					bind = new Bind(splits[0], widget, splits[1], es.current().key, converter);
				}
				else
				{
					Log::E("Bind Source '" + source + "' unrecognized");
					continue;
				}
			}
			else
			{
				bind = new Bind(widget, bindSourcePath, es.current().key, converter);
			}
			widget->AddBind(bind);
		}
		else
			widget->SetPropertyValue(es.current().key, es.current().value);
	}

	Dictionary<Str, XMLNode*>::Enumerator ec = node->complexAttributes.getBegin();
	while (ec.next())
	{
		widget->SetPropertyValue(ec.current().key, ec.current().value);
	}

	for(int32_t i=0; i<node->children.GetSize(); i++)
		LoadWidget(node->children[i], widget->GetContentWidget());

	return true;
}

Widget* XWMLReader::CreateWidgetByName(Str name, Widget* parent)
{
	if (name == "Widget")
		return new Widget(parent);
	if (name == "Button")
		return new Button(parent);
	if (name == "Label")
		return new Label(parent);
	if (name == "StackLayout")
		return new StackLayout(parent);
	if (name == "ListView")
		return new ListView(parent);
	if (name == "TextBox")
		return new TextBox(parent);
	if (name == "TreeView")
		return new TreeView(parent);
	if (name == "ImageWidget")
		return new ImageWidget(parent);

	return NULL;
}

} // namespace crown

