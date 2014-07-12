cmake_minimum_required(VERSION 2.8)

if (LINUX)
	install (FILES     luajit/${CROWN_ARCH}/lib/libluajit-5.1.so.2.0.3 DESTINATION bin)
	install (FILES     luajit/${CROWN_ARCH}/lib/libluajit-5.1.so.2 DESTINATION bin)
	install (FILES     luajit/${CROWN_ARCH}/bin/luajit DESTINATION bin)
	install (PROGRAMS  luajit/${CROWN_ARCH}/bin/luajit-2.0.3 DESTINATION bin)
	install (DIRECTORY luajit/${CROWN_ARCH}/share/luajit-2.0.3/jit DESTINATION bin)

	set (BX_DIR bx)
	set (BGFX_DIR bgfx)
	set (FCPP_DIR ${BGFX_DIR}/3rdparty/fcpp)
	set (GLSL_OPTIMIZER ${BGFX_DIR}/3rdparty/glsl-optimizer)

	# BGFX
	add_library(bgfx STATIC
		${BGFX_DIR}/src/bgfx.cpp
		${BGFX_DIR}/src/glcontext_egl.cpp
		${BGFX_DIR}/src/glcontext_glx.cpp
		${BGFX_DIR}/src/glcontext_ppapi.cpp
		${BGFX_DIR}/src/glcontext_wgl.cpp
		${BGFX_DIR}/src/image.cpp
		${BGFX_DIR}/src/renderer_d3d11.cpp
		${BGFX_DIR}/src/renderer_d3d9.cpp
		${BGFX_DIR}/src/renderer_gl.cpp
		${BGFX_DIR}/src/renderer_null.cpp
		${BGFX_DIR}/src/vertexdecl.cpp
	)

	target_include_directories(bgfx PRIVATE
		${BX_DIR}/include

		${BGFX_DIR}/include
		${BGFX_DIR}/src
		${BGFX_DIR}/3rdparty/khronos
	)

	target_compile_options(bgfx PRIVATE
		-fPIC
	)

	# Texture Compiler
	add_executable(texturec
		${BGFX_DIR}/src/image.cpp
		${BGFX_DIR}/tools/texturec/texturec.cpp
	)

	target_include_directories(texturec PRIVATE
		${BX_DIR}/include
		${BGFX_DIR}/include
		${BGFX_DIR}/src
	)

	# Shader Compiler
	file (GLOB GLSL_OPTIMIZER_SRC
		${GLSL_OPTIMIZER}/src/mesa/*.c
		${GLSL_OPTIMIZER}/src/glsl/*.c
		${GLSL_OPTIMIZER}/src/glsl/*.cpp
	)

	add_executable(shaderc
		${BGFX_DIR}/tools/shaderc/shaderc.cpp

		${FCPP_DIR}/cpp1.c
		${FCPP_DIR}/cpp2.c
		${FCPP_DIR}/cpp3.c
		${FCPP_DIR}/cpp4.c
		${FCPP_DIR}/cpp5.c
		${FCPP_DIR}/cpp6.c
		${FCPP_DIR}/cpp6.c

		${GLSL_OPTIMIZER}/src/glsl/ast_array_index.cpp
		${GLSL_OPTIMIZER}/src/glsl/ast_expr.cpp
		${GLSL_OPTIMIZER}/src/glsl/ast_function.cpp
		${GLSL_OPTIMIZER}/src/glsl/ast_to_hir.cpp
		${GLSL_OPTIMIZER}/src/glsl/ast_type.cpp
		${GLSL_OPTIMIZER}/src/glsl/builtin_functions.cpp
		${GLSL_OPTIMIZER}/src/glsl/builtin_types.cpp
		${GLSL_OPTIMIZER}/src/glsl/builtin_variables.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_lexer.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_optimizer.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_parser.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_parser_extras.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_symbol_table.cpp
		${GLSL_OPTIMIZER}/src/glsl/glsl_types.cpp
		${GLSL_OPTIMIZER}/src/glsl/hir_field_selection.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_basic_block.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_builder.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_clone.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_constant_expression.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_equals.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_expression_flattening.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_function_can_inline.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_function.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_function_detect_recursion.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_hierarchical_visitor.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_hv_accept.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_import_prototypes.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_print_glsl_visitor.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_print_visitor.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_rvalue_visitor.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_stats.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_unused_structs.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_validate.cpp
		${GLSL_OPTIMIZER}/src/glsl/ir_variable_refcount.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_atomics.cpp
		${GLSL_OPTIMIZER}/src/glsl/linker.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_functions.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_interface_blocks.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_uniform_block_active_visitor.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_uniform_blocks.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_uniform_initializers.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_uniforms.cpp
		${GLSL_OPTIMIZER}/src/glsl/link_varyings.cpp
		${GLSL_OPTIMIZER}/src/glsl/loop_analysis.cpp
		${GLSL_OPTIMIZER}/src/glsl/loop_controls.cpp
		${GLSL_OPTIMIZER}/src/glsl/loop_unroll.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_clip_distance.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_discard.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_discard_flow.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_if_to_cond_assign.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_instructions.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_jumps.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_mat_op_to_vec.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_named_interface_blocks.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_noise.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_output_reads.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_packed_varyings.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_packing_builtins.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_ubo_reference.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_variable_index_to_cond_assign.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_cond_assign.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_swizzle.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_vector.cpp
		${GLSL_OPTIMIZER}/src/glsl/lower_vector_insert.cpp
		# ${GLSL_OPTIMIZER}/src/glsl/main.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_algebraic.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_array_splitting.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_constant_folding.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_constant_propagation.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_constant_variable.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation_elements.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_cse.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_dead_builtin_varyings.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_dead_code.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_dead_code_local.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_dead_functions.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_flatten_nested_if_blocks.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_flip_matrices.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_function_inlining.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_if_simplification.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_noop_swizzle.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_redundant_jumps.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_structure_splitting.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_swizzle_swizzle.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_tree_grafting.cpp
		${GLSL_OPTIMIZER}/src/glsl/opt_vectorize.cpp
		${GLSL_OPTIMIZER}/src/glsl/s_expression.cpp
		${GLSL_OPTIMIZER}/src/glsl/standalone_scaffolding.cpp

		${GLSL_OPTIMIZER}/src/glsl/ralloc.c
		${GLSL_OPTIMIZER}/src/glsl/strtod.c

		${GLSL_OPTIMIZER}/src/mesa/main/hash_table.c
		${GLSL_OPTIMIZER}/src/mesa/main/imports.c

		${GLSL_OPTIMIZER}/src/mesa/program/prog_hash_table.c
		${GLSL_OPTIMIZER}/src/mesa/program/symbol_table.c

		${GLSL_OPTIMIZER}/src/glsl/glcpp/pp.c
		${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-lex.c
		${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-parse.c		
	)

	target_include_directories (shaderc PRIVATE
		${BX_DIR}/include

		${FCPP_DIR}

		${GLSL_OPTIMIZER}/include
		${GLSL_OPTIMIZER}/src/mesa
		${GLSL_OPTIMIZER}/src/mapi
		${GLSL_OPTIMIZER}/src/glsl
	)

	target_compile_options (shaderc PRIVATE
		-std=c++11
		-fno-strict-aliasing
	)

	install (TARGETS texturec DESTINATION bin)
	install (TARGETS shaderc DESTINATION bin)

elseif (WINDOWS)
	install (FILES     luajit/${CROWN_ARCH}/lib/lua51.dll DESTINATION bin)
	install (FILES     luajit/${CROWN_ARCH}/lib/luajit-5.1.dll DESTINATION bin)
	install (PROGRAMS  luajit/${CROWN_ARCH}/bin/luajit.exe DESTINATION bin)
	install (DIRECTORY luajit/${CROWN_ARCH}/share/luajit-2.0.1/jit DESTINATION bin)

	install (FILES     openAL/lib/OpenAL32.dll DESTINATION bin)
else ()
	message (FATAL_ERROR "Wrong build configuration")
endif (LINUX)
