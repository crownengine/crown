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
#include "WindowsManager.h"
#include "XMLReader.h"

namespace Crown
{

class WindowContext;

class XWMLReader
{
public:
  XWMLReader();
  ~XWMLReader();

  Window* LoadFile(Str filePath, WindowsManager* windowsManager, WindowContext* context);
	static bool ParseAction(const Str& actionStr, Str& actionName, List<Str>* arguments);
	static bool ParseBind(const Str& bindStr, Str& bindPath, Str& source, Str& converter);

	static bool LoadWidgetPropertiesAndChildren(XMLNode* node, Widget* widget);

private:
	static Widget* LoadWidget(XMLNode* node, Widget* parent);
		static Window* LoadWindow(XMLNode* node, WindowsManager* windowsManager);
	static Widget* CreateWidgetByName(Str name, Widget* parent);
};

} // namespace Crown

