# LuaJIT CMake build file
cmake_minimum_required (VERSION 2.8)

project (crown-luajit)
	
set (LUAJIT_SOURCE_NAME "LuaJIT-2.0.1")

	
set(LUAJIT_SRC_FOLDER "src")
# Create variable for include and library
set(LUAJIT_INCLUDE_DIR "${LUAJIT_SRC_FOLDER}" CACHE STRING "LuaJIT include directory" FORCE)
set(LUAJIT_LIBRARIES LUAJIT CACHE STRING "LuaJIT libraries" FORCE)

set (LUA_SRC
    ${LUAJIT_SRC_FOLDER}/lib_aux.c
    ${LUAJIT_SRC_FOLDER}/lib_base.c
    ${LUAJIT_SRC_FOLDER}/lib_bit.c
    ${LUAJIT_SRC_FOLDER}/lib_debug.c
    ${LUAJIT_SRC_FOLDER}/lib_ffi.c
    ${LUAJIT_SRC_FOLDER}/lib_init.c
    ${LUAJIT_SRC_FOLDER}/lib_io.c
    ${LUAJIT_SRC_FOLDER}/lib_jit.c
    ${LUAJIT_SRC_FOLDER}/lib_math.c
    ${LUAJIT_SRC_FOLDER}/lib_os.c
    ${LUAJIT_SRC_FOLDER}/lib_package.c
    ${LUAJIT_SRC_FOLDER}/lib_string.c
    ${LUAJIT_SRC_FOLDER}/lib_table.c
    ${LUAJIT_SRC_FOLDER}/lj_alloc.c
    ${LUAJIT_SRC_FOLDER}/ljamalg.c
    ${LUAJIT_SRC_FOLDER}/lj_api.c
    ${LUAJIT_SRC_FOLDER}/lj_asm.c
    ${LUAJIT_SRC_FOLDER}/lj_bc.c
    ${LUAJIT_SRC_FOLDER}/lj_bcread.c
    ${LUAJIT_SRC_FOLDER}/lj_bcwrite.c
    ${LUAJIT_SRC_FOLDER}/lj_carith.c
    ${LUAJIT_SRC_FOLDER}/lj_ccall.c
    ${LUAJIT_SRC_FOLDER}/lj_ccallback.c
    ${LUAJIT_SRC_FOLDER}/lj_cconv.c
    ${LUAJIT_SRC_FOLDER}/lj_cdata.c
    ${LUAJIT_SRC_FOLDER}/lj_char.c
    ${LUAJIT_SRC_FOLDER}/lj_clib.c
    ${LUAJIT_SRC_FOLDER}/lj_cparse.c
    ${LUAJIT_SRC_FOLDER}/lj_crecord.c
    ${LUAJIT_SRC_FOLDER}/lj_ctype.c
	${LUAJIT_SRC_FOLDER}/lj_debug.c
    ${LUAJIT_SRC_FOLDER}/lj_dispatch.c
    ${LUAJIT_SRC_FOLDER}/lj_err.c
    ${LUAJIT_SRC_FOLDER}/lj_ffrecord.c
    ${LUAJIT_SRC_FOLDER}/lj_func.c
    ${LUAJIT_SRC_FOLDER}/lj_gc.c
    ${LUAJIT_SRC_FOLDER}/lj_gdbjit.c
    ${LUAJIT_SRC_FOLDER}/lj_ir.c
    ${LUAJIT_SRC_FOLDER}/lj_lex.c
    ${LUAJIT_SRC_FOLDER}/lj_lib.c
    ${LUAJIT_SRC_FOLDER}/lj_load.c
    ${LUAJIT_SRC_FOLDER}/lj_mcode.c
    ${LUAJIT_SRC_FOLDER}/lj_meta.c
    ${LUAJIT_SRC_FOLDER}/lj_obj.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_dce.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_fold.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_loop.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_mem.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_narrow.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_sink.c
    ${LUAJIT_SRC_FOLDER}/lj_opt_split.c
    ${LUAJIT_SRC_FOLDER}/lj_parse.c
    ${LUAJIT_SRC_FOLDER}/lj_record.c
    ${LUAJIT_SRC_FOLDER}/lj_snap.c
    ${LUAJIT_SRC_FOLDER}/lj_state.c
    ${LUAJIT_SRC_FOLDER}/lj_str.c
    ${LUAJIT_SRC_FOLDER}/lj_strscan.c
    ${LUAJIT_SRC_FOLDER}/lj_tab.c
    ${LUAJIT_SRC_FOLDER}/lj_trace.c
    ${LUAJIT_SRC_FOLDER}/lj_udata.c
    ${LUAJIT_SRC_FOLDER}/lj_vmevent.c
    ${LUAJIT_SRC_FOLDER}/lj_vmmath.c
    ${LUAJIT_SRC_FOLDER}/luajit.c
)

set (LUA_HEADERS
    ${LUAJIT_SRC_FOLDER}/lj_alloc.h
    ${LUAJIT_SRC_FOLDER}/lj_arch.h	
    ${LUAJIT_SRC_FOLDER}/lj_asm.h	
    ${LUAJIT_SRC_FOLDER}/lj_asm_arm.h
    ${LUAJIT_SRC_FOLDER}/lj_asm_mips.h
    ${LUAJIT_SRC_FOLDER}/lj_asm_ppc.h
    ${LUAJIT_SRC_FOLDER}/lj_asm_x86.h
    ${LUAJIT_SRC_FOLDER}/lj_bc.h	
    ${LUAJIT_SRC_FOLDER}/lj_bcdump.h
    ${LUAJIT_SRC_FOLDER}/lj_carith.h
    ${LUAJIT_SRC_FOLDER}/lj_ccall.h
    ${LUAJIT_SRC_FOLDER}/lj_ccallback.h
    ${LUAJIT_SRC_FOLDER}/lj_cconv.h
    ${LUAJIT_SRC_FOLDER}/lj_cdata.h
    ${LUAJIT_SRC_FOLDER}/lj_char.h
    ${LUAJIT_SRC_FOLDER}/lj_clib.h
    ${LUAJIT_SRC_FOLDER}/lj_cparse.h
    ${LUAJIT_SRC_FOLDER}/lj_crecord.h
    ${LUAJIT_SRC_FOLDER}/lj_ctype.h
    ${LUAJIT_SRC_FOLDER}/lj_debug.h
    ${LUAJIT_SRC_FOLDER}/lj_def.h
    ${LUAJIT_SRC_FOLDER}/lj_dispatch.h
    ${LUAJIT_SRC_FOLDER}/lj_emit_arm.h
    ${LUAJIT_SRC_FOLDER}/lj_emit_mips.h
    ${LUAJIT_SRC_FOLDER}/lj_emit_ppc.h
    ${LUAJIT_SRC_FOLDER}/lj_emit_x86.h
    ${LUAJIT_SRC_FOLDER}/lj_err.h
    ${LUAJIT_SRC_FOLDER}/lj_errmsg.h
    ${LUAJIT_SRC_FOLDER}/lj_ff.h
    ${LUAJIT_SRC_FOLDER}/lj_ffrecord.h
    ${LUAJIT_SRC_FOLDER}/lj_frame.h
    ${LUAJIT_SRC_FOLDER}/lj_func.h
    ${LUAJIT_SRC_FOLDER}/lj_gc.h
    ${LUAJIT_SRC_FOLDER}/lj_gdbjit.h
    ${LUAJIT_SRC_FOLDER}/lj_ir.h
    ${LUAJIT_SRC_FOLDER}/lj_ircall.h
    ${LUAJIT_SRC_FOLDER}/lj_iropt.h
    ${LUAJIT_SRC_FOLDER}/lj_jit.h
    ${LUAJIT_SRC_FOLDER}/lj_lex.h
    ${LUAJIT_SRC_FOLDER}/lj_lib.h
    ${LUAJIT_SRC_FOLDER}/lj_mcode.h
    ${LUAJIT_SRC_FOLDER}/lj_meta.h
    ${LUAJIT_SRC_FOLDER}/lj_obj.h
    ${LUAJIT_SRC_FOLDER}/lj_parse.h
    ${LUAJIT_SRC_FOLDER}/lj_record.h
    ${LUAJIT_SRC_FOLDER}/lj_snap.h
    ${LUAJIT_SRC_FOLDER}/lj_state.h
    ${LUAJIT_SRC_FOLDER}/lj_str.h
    ${LUAJIT_SRC_FOLDER}/lj_strscan.h
    ${LUAJIT_SRC_FOLDER}/lj_tab.h
    ${LUAJIT_SRC_FOLDER}/lj_target.h
    ${LUAJIT_SRC_FOLDER}/lj_target_arm.h
    ${LUAJIT_SRC_FOLDER}/lj_target_mips.h
    ${LUAJIT_SRC_FOLDER}/lj_target_ppc.h
    ${LUAJIT_SRC_FOLDER}/lj_target_x86.h
    ${LUAJIT_SRC_FOLDER}/lj_trace.h
    ${LUAJIT_SRC_FOLDER}/lj_traceerr.h
    ${LUAJIT_SRC_FOLDER}/lj_udata.h
    ${LUAJIT_SRC_FOLDER}/lj_vm.h
    ${LUAJIT_SRC_FOLDER}/lj_vmevent.h
    ${LUAJIT_SRC_FOLDER}/luaconf.h
    ${LUAJIT_SRC_FOLDER}/lua.hpp
)

set (SOURCES
    ${LUA_SRC}
    ${LUA_HEADERS}
)

include_directories("${LUAJIT_SOURCE_NAME}/dynasm")

add_library(crown-luajit STATIC ${SOURCES})


#set_target_properties (${LUAJIT_LIBRARIES} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LUAJIT_STATICLIB_OUTPUT_PATH}")
	
#if (WINDOWS)
#	if (MSVC)
#	    set(lua_compile_def
#       	_CRT_SECURE_NO_DEPRECATE
#           _CRT_NONSTDC_NO_DEPRECATE)
#   endif (MSVC)
#endif (WINDOWS)
#set_target_properties(${LUAJIT_LIBRARIES} PROPERTIES
#COMPILE_DEFINITIONS "${lua_compile_def}")

