/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_WINDOWS
#include "core/debug/callstack.h"
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#pragma warning(push)
#pragma warning(disable:4091) // 'keyword' : ignored on left of 'type' when no variable is declared
#include <dbghelp.h>
#pragma warning(pop)
#include <new>

namespace crown
{
namespace debug
{
	s32 callstack_init()
	{
		return 0;
	}

	void callstack_shutdown()
	{
		CE_NOOP();
	}

	void callstack(log_internal::System system, LogSeverity::Enum severity)
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
#if defined(_M_IX86)
		mtype = IMAGE_FILE_MACHINE_I386;
		stack.AddrPC.Offset = ctx.Eip;
		stack.AddrPC.Mode = AddrModeFlat;
		stack.AddrFrame.Offset = ctx.Ebp;
		stack.AddrFrame.Mode = AddrModeFlat;
		stack.AddrStack.Offset = ctx.Esp;
		stack.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_X64)
		mtype = IMAGE_FILE_MACHINE_AMD64;
		stack.AddrPC.Offset = ctx.Rip;
		stack.AddrPC.Mode = AddrModeFlat;
		stack.AddrFrame.Offset = ctx.Rsp;
		stack.AddrFrame.Mode = AddrModeFlat;
		stack.AddrStack.Offset = ctx.Rsp;
		stack.AddrStack.Mode = AddrModeFlat;
#endif // if defined(_M_IX86)

		DWORD ldsp = 0;
		IMAGEHLP_LINE64 line;
		ZeroMemory(&line, sizeof(IMAGEHLP_LINE64));
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		char buf[sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(TCHAR)];
		SYMBOL_INFO *sym = new (buf) SYMBOL_INFO();
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
			)) {
			if (stack.AddrPC.Offset == 0)
				break;

			++num;

			BOOL res = SymGetLineFromAddr64(GetCurrentProcess()
				, stack.AddrPC.Offset
				, &ldsp
				, &line
				);
			res = res && SymFromAddr(GetCurrentProcess(), stack.AddrPC.Offset, 0, sym);

			if (res == TRUE) {
				log_internal::logx(severity
					, system
					, "[%2i] %s in %s:%d"
					, num
					, sym->Name
					, line.FileName
					, line.LineNumber
					);
			} else {
				log_internal::logx(severity
					, system
					, "[%2i] 0x%p"
					, num
					, stack.AddrPC.Offset
					);
			}
		}

		SymCleanup(GetCurrentProcess());
	}

} // namespace debug

} // namespace crown

#endif // if CROWN_PLATFORM_WINDOWS
