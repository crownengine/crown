/*
 * Copyright (c) 2012-2026 Daniele Bartolini <dbartolini@crownengine.org>
 * SPDX-License-Identifier: MIT
 */

[CCode (cheader_filename = "tinyexpr.h")]
namespace TinyExpr
{
[CCode (cname = "", cprefix = "TE_", has_type_id = false)]
public enum VariableType
{
	VARIABLE,

	FUNCTION0,
	FUNCTION1,
	FUNCTION2,
	FUNCTION3,
	FUNCTION4,
	FUNCTION5,
	FUNCTION6,
	FUNCTION7,

	CLOSURE0,
	CLOSURE1,
	CLOSURE2,
	CLOSURE3,
	CLOSURE4,
	CLOSURE5,
	CLOSURE6,
	CLOSURE7,

	FLAG_PURE
}

[Compact]
[CCode (cname = "te_expr", free_function = "te_free")]
public class Expr
{
	int type;
	[CCode (cname = "value")]
	double value;
	[CCode (cname = "bound")]
	double* bound;
	[CCode (cname = "function")]
	void* function;
	void* parameters[1];
}

[SimpleType]
[CCode (cname = "te_variable", has_type_id = false)]
public struct Variable
{
	char* name;
	void* address;
	int type;
	void* context;
}

[CCode (cname = "te_interp")]
double interp(string expression, out int error);

[CCode (cname = "te_compile")]
Expr compile(string expression, Variable[] variables, out int error);

[CCode (cname = "te_eval")]
double eval(Expr n);

[CCode (cname = "te_print")]
void print(Expr n);

} /* namespace TinyExpr */
