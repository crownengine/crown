/// This file implements a small language for computing arithmetic expressions,
/// such as (a + b)*3 - sin(y). It allows you freedom in specifying variable
/// dependencies, while avoiding the complexities of a full blown scripting
/// language.
///
/// The language allows for constants (which are converted to numbers at compile
/// time) and variables (which are evaluated as numbers during run time).
/// The language has a fixed small set of operators and functions (more are easy
/// to add). All constants, variables and stack values used by the language are
/// 32 bit floats.
///
/// During the compilation phase, the source string is compiled to byte code
/// which is run during the run phase. The virtual machine is a simple stack-based
/// machine with the stack explicitly supplied by the caller.
///
/// All instructions in the byte code are 32-bit wide. The instruction is either
/// a regular float in which case the operation is BC_PUSH_FLOAT -- the float is
/// pushed to the stack, or it is a NaN. If the float is a NaN, the mantissa is
/// used to encode the type of operation and additional data. The first three bits
/// encode the operation and the remaining 20 bits encode the operation data.
///
/// NAN_MARKER (9) BC_PUSH_VAR (3)	id (20)		Pushes the variable with the specified id.
///	NAN_MARKER (9) BC_FUNCTION (3)	id (20)		Computes the function with the specified id.
/// NAN_MARKER (9) BC_END (3)	    zero (20)	Marks the end of the byte code.
/// float (32)									Pushes the float.

/// Flag used to include the parts of the code needed to compile to bytecode.
/// If you compile offline you can exclude this code in the runtime version.
#define CAN_COMPILE

namespace crown
{
namespace skinny
{
namespace expression_language
{
#ifdef CAN_COMPILE
		/// Compiles the @a source and stores the result in the @a byte_code.
		/// @a variables is a list of variable names. The position of the variable in
		/// the list should match the position when @a variables is sent to the run
		/// function.
		///
		/// @a constants and @a constant_values specifies a list of runtime constants
		/// and corresponding values. Constants are expanded to numbers at compile
		/// time.
		///
		/// Returns the number of compiled unsigned words. If the returned number is
		/// greater than @a byte_code_capacity, only the first @a byte_code_capacity
		/// words of the byte code are written to @a byte_code.
		unsigned compile(const char *source
			, unsigned num_variables
			, const char **variables
			, unsigned num_constants
			, const char **constants
			, const float *constant_values
			, unsigned *byte_code
			, unsigned byte_code_capacity
			);
#endif

	/// Returns true if the byte code is constant. I. e., it always produces the same float value.
	bool is_constant(const unsigned *byte_code);

	/// Returns the constant value produced by the byte code.
	float constant_value(const unsigned *byte_code);

	/// Represents the working stack.
	struct Stack
	{
		float *data;
		unsigned size;
		unsigned capacity;

		Stack(float *data, unsigned capacity)
			: data(data)
			, size(0)
			, capacity(capacity)
		{
		}
	};

	/// Runs the @a byte_code using the @a stack as execution stack.
	/// @a variables is a list of variable values to use for the execution.
	/// They should match the list of variable names supplied to the compile function.
	bool run(const unsigned *byte_code, const float *variables, Stack &stack);

} // namespace expression_language

} // namespace skinny

} // namespace crown
