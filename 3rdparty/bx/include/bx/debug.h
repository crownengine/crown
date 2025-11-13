/*
 * Copyright 2010-2025 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_DEBUG_H_HEADER_GUARD
#define BX_DEBUG_H_HEADER_GUARD

#include <stdint.h> // uint32_t
#include <stdarg.h> // va_list

namespace bx
{
	class  Error;
	class  StringView;
	struct WriterI;

	/// Break in debugger.
	///
	void debugBreak();

	/// Write string to debug output.
	///
	/// @param[in] _str Zero terminated string to write.
	///
	void debugOutput(const char* _str);

	/// Write string to debug output.
	///
	/// @param[in] _str StringView to write.
	///
	void debugOutput(const StringView& _str);

	/// Write formatted string to debug output.
	///
	void debugPrintfVargs(const char* _format, va_list _argList);

	/// Write formatted string to debug output.
	///
	void debugPrintf(const char* _format, ...);

	/// Write hex data into debug output.
	///
	void debugPrintfData(const void* _data, uint32_t _size, const char* _format, ...);

	/// Return debug output writer.
	///
	/// @returns Debug output writer.
	///
	WriterI* getDebugOut();

	/// Capture current callstack fast.
	///
	/// @param[in]  _skip Skip top N stack frames.
	/// @param[in]  _max Maximum frame to capture.
	/// @param[out] _outStack Stack frames array. Must be at least `_max` elements.
	///
	/// @returns Number of stack frames captured.
	///
	uint32_t getCallStackFast(uint32_t _skip, uint32_t _max, uintptr_t* _outStack);

	/// Capture current callstack with slower but more accurate method.
	///
	/// @param[in]  _skip Skip top N stack frames.
	/// @param[in]  _max Maximum frame to capture.
	/// @param[out] _outStack Stack frames array. Must be at least `_max` elements.
	///
	/// @returns Number of stack frames captured.
	///
	uint32_t getCallStackExact(uint32_t _skip, uint32_t _max, uintptr_t* _outStack);

	/// Write callstack.
	///
	/// @param[in]  _writer Writer.
	/// @param[in]  _stack Callstack.
	/// @param[in]  _num Number of stack addresses in `_stack` array.
	/// @param[out] _err Error.
	///
	/// @returns Number of bytes writen to `_writer`.
	///
	int32_t writeCallstack(WriterI* _writer, const uintptr_t* _stack, uint32_t _num, Error* _err);

	/// Capture call stack with `bx::getCallStackExact`, and write it to debug output.
	///
	/// @param[in] _skip Skip top N stack frames.
	///
	void debugOutputCallstack(uint32_t _skip);

	/// Install BX exception handler.
	void installExceptionHandler();

} // namespace bx

#endif // BX_DEBUG_H_HEADER_GUARD
