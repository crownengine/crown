/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include "log.h"
#include "win_headers.h"
#include <dbghelp.h>

namespace crown
{
namespace error
{
	void print_callstack()
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
		while (StackWalk64(mtype
				, GetCurrentProcess()
				, GetCurrentThread()
				, &stack
				, &ctx
				, NULL
				, SymFunctionTableAccess64
				, SymGetModuleBase64
				, NULL
				))
		{
			if (stack.AddrPC.Offset == 0)
				break;

			++num;

			BOOL res = SymGetLineFromAddr64(GetCurrentProcess()
						, stack.AddrPC.Offset
						, &ldsp
						, &line
						);
			res = res && SymFromAddr(GetCurrentProcess(), stack.AddrPC.Offset, 0, sym);

			if (res == TRUE)
				logi("\t[%2i] %s in %s:%d", num, sym->Name, line.FileName, line.LineNumber);
			else
				logi("\t[%2i] 0x%p", num, stack.AddrPC.Offset);
		}

		SymCleanup(GetCurrentProcess());
	}
} // namespace error

} // namespace crown

#endif // CROWN_PLATFORM_WINDOWS
