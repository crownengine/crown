/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

namespace crown
{

void stacktrace()
{
	SymInitialize(GetCurrentProcess(), NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	DWORD mtype;
	CONTEXT ctx;
	ZeroMemory(&ctx, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_CONTROL;
	RtlCaptureContext(&ctx);

	STACKFRAME64 stack;
	ZeroMemory(&stack, sizeof(STACKFRAME64));
#ifdef _M_IX86
	mtype = IMAGE_FILE_MACHINE_I386;
	stack.AddrPC.Offset = ctx.Eip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = ctx.Ebp;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Offset = ctx.Esp;
	stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	mtype = IMAGE_FILE_MACHINE_AMD64;
	stack.AddrPC.Offset = ctx.Rip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = ctx.Rsp;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Offset = ctx.Rsp;
	stack.AddrStack.Mode = AddrModeFlat;
#endif

	DWORD ldsp = 0;
	IMAGEHLP_LINE64 line;
	ZeroMemory(&line, sizeof(IMAGEHLP_LINE64));
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO sym = (PSYMBOL_INFO)buf;
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = MAX_SYM_NAME;

	UINT num = 0;
	while (StackWalk64(mtype,
			GetCurrentProcess(),
			GetCurrentThread(),
			&stack,
			&ctx,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL))
	{

		if (stack.AddrPC.Offset == 0)
			break;

		++num;
		BOOL res = SymGetLineFromAddr64(GetCurrentProcess(), stack.AddrPC.Offset, &ldsp, &line) &&
			SymFromAddr(GetCurrentProcess(), stack.AddrPC.Offset, 0, sym);

		if (res == TRUE)
			printf("\t[%i] %s (%s:%d)\n", num, sym->Name, line.FileName, line.LineNumber);
		else
			printf("\t[%i] 0x%p\n", num, stack.AddrPC.Offset);
	}

	SymCleanup(GetCurrentProcess());
}
} // namespace crown

#endif // CROWN_PLATFORM_WINDOWS
