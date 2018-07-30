#include "core/error/error.h"
#include "core/math/math.h"
#include "core/strings/string.h"
#include "resource/expression_language.h"
#include <alloca.h>
#include <limits.h> // UINT_MAX
#include <stdlib.h> // strtof
#include <string.h> // memmove

namespace crown
{
namespace skinny
{
namespace expression_language
{
	/// Byte code constants.
	///
	/// If the upper 12 bits of the byte code do not match one of these values, the operation is
	/// BC_PUSH_FLOAT and the byte code specify the 32 bit float to push. If the upper 12 bits
	/// match one of these values (which are all NaNs, so they should never appear as regular
	/// floats), the operation will instead be the one matching.
	///
	/// The remaining 20 bits of the byte code are used for the id of functions and variables.
	enum ByteCode {BC_FUNCTION = 0x7f800000, BC_PUSH_VAR = 0x7f900000, BC_END = 0x7fa00000};

	/// Returns the byte code operation part of the byte code word.
	static inline unsigned bc_mask(unsigned i) {return i & 0xfff00000;}

	/// Returns the id part of the byte code word.
	static inline unsigned id_mask(unsigned i) {return i & 0x000fffff;}

	/// Opcodes for functions
	enum OpCode
	{
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_UNARY_MINUS,
		OP_NOP,
		OP_SIN,
		OP_COS,
		OP_ABS,
		OP_MATCH,
		OP_MATCH_2D
	};

	static inline float pop(Stack &stack) 			{CE_ASSERT(stack.size > 0, "Stack underflow"); return stack.data[--stack.size];}
	static inline void push(Stack &stack, float f)	{CE_ASSERT(stack.size < stack.capacity, "Stack overflow"); stack.data[stack.size++] = f;}

	inline float fmax(float a, float b)
	{
		return a > b ? a : b;
	}

	inline float length(float a, float b)
	{
		return fsqrt((b - a) * (b - a));
	}

	inline float match(float a, float b)
	{
		return fmax(1.0f-length(a, b), 0.0f);
	}

	inline float match2d(float a, float b, float c, float d)
	{
		return match(a, b) * match(c, d);
	}

	/// Computes the function specified by @a op_code on the @a stack.
	static inline void compute_function(OpCode op_code, Stack &stack)
	{
		#define POP() pop(stack)
		#define PUSH(f) push(stack, f)

		float a,b,c,d;

		switch(op_code) {
			case OP_ADD: b=POP(); a=POP(); PUSH(a+b); break;
			case OP_SUB: b=POP(); a=POP(); PUSH(a-b); break;
			case OP_MUL: b=POP(); a=POP(); PUSH(a*b); break;
			case OP_DIV: b=POP(); a=POP(); PUSH(a/b); break;
			case OP_UNARY_MINUS: PUSH(-POP()); break;
			case OP_SIN: PUSH(fsin(POP())); break;
			case OP_COS: PUSH(fcos(POP())); break;
			case OP_ABS: a = POP(); PUSH(fabs(a)); break;
			case OP_MATCH: b=POP(); a=POP(); PUSH(match(a, b)); break;
			case OP_MATCH_2D: d=POP(); c=POP(); b=POP(); a=POP(); PUSH(match2d(a,b,c,d)); break;
			case OP_NOP: break;
			default:
				CE_FATAL("Unknown opcode");
		}

		#undef POP
		#undef PUSH
	}

	/// Union to cast through to convert between float and unsigned.
	union FloatAndUnsigned
	{
		float f;
		unsigned u;
	};

	static inline float unsigned_to_float(unsigned u)	{FloatAndUnsigned fu; fu.u = u; return fu.f;}

	#ifdef CAN_COMPILE

	static inline unsigned float_to_unsigned(float f)	{FloatAndUnsigned fu; fu.f = f; return fu.u;}

	#ifdef WIN32
		float strtof(const char *nptr, char **endptr) {return (float)strtod(nptr, endptr);}
	#endif

	/// Represents a token in the expression language. The tokens are used
	/// both during the tokenization phase and as a representation of the
	/// program. (A list of tokens in reverse polish notation form.)
	struct Token
	{
		enum TokenType {EMPTY, NUMBER, FUNCTION, VARIABLE, LEFT_PARENTHESIS, RIGHT_PARENTHESIS};

		Token() : type(EMPTY), id(0) {}
		Token(TokenType type) : type(type) {}
		Token(TokenType type, unsigned id) : type(type), id(id) {}
		Token(TokenType type, float value) : type(type), value(value) {}

		TokenType type;		///< Identifies the type of the token
		union {
			unsigned id;		///< Id for FUNCTION and VARIABLE tokens
			float value;		///< Numeric value for NUMBER tokens
		};
	};

	/// Describes a function.
	struct Function
	{
		Function() {}
		Function(OpCode op_code, unsigned precedence, unsigned arity) : op_code(op_code), precedence(precedence), arity(arity) {}

		OpCode 		op_code;	///< The opcode of the function
		unsigned 	precedence;	///< The precedence of the function operator.
		unsigned 	arity;		///< The number of arguments that the function takes.
	};


	/// Represents the environment in which we are compiling -- the available variables,
	/// constants and functions.
	struct CompileEnvironment
	{
		unsigned num_variables;
		const char **variable_names;
		unsigned num_constants;
		const char **constant_names;
		const float *constant_values;
		unsigned num_functions;
		const char **function_names;
		const Function *function_values;

		/// Finds a string in @a strings matching @a s of length @a len and returns its index.
		/// Returns UINT_MAX if no such string is found.
		static unsigned find_string(const char *s, unsigned len, unsigned num_strings, const char **strings)
		{
			for (unsigned i=0; i<num_strings; ++i)
				if (strncmp(s, strings[i], len) == 0 && strlen32(strings[i]) == len)
					return i;
			return UINT_MAX;
		}

		/// Finds a token representing the identifier in the environment.
		Token token_for_identifier(const char *identifier, unsigned len) const
		{
			unsigned i;
			if ( (i=find_string(identifier, len, num_variables, variable_names)) != UINT_MAX)
				return Token(Token::VARIABLE, i);
			else if ( (i=find_string(identifier, len, num_constants, constant_names)) != UINT_MAX)
				return Token(Token::NUMBER, constant_values[i]);
			else if ( (i=find_string(identifier, len, num_functions, function_names)) != UINT_MAX)
				return Token(Token::FUNCTION, i);
			else {
				CE_FATAL("Unknown identifier: %s", identifier);
				return Token();
			}
		}

		/// Finds a token representing the identifier in the environment.
		Token token_for_identifier(const char *identifier) const
		{
			return token_for_identifier(identifier, strlen32(identifier));
		}

		/// True if there is a function matching the specified identifier.
		bool has_function(char * identifier) const
		{
			return find_string(identifier, strlen32(identifier), num_functions, function_names) != UINT_MAX;
		}
	};

	/// Tokenizes the source code @a p into a sequence of tokens. The environment @a env
	/// is used for looking up source code identifiers.
	/// Returns the total number of tokens. If the returned number is greater than the @a capcity, only
	/// the first @a capacity items will be converted.
	static unsigned tokenize(const char *p, const CompileEnvironment &env, Token *tokens, unsigned capacity)
	{
		// Determines if the next + or - is a binary or unary operator.
		bool binary = false;

		unsigned num_tokens = 0;
		unsigned overflow_tokens = 0;

		while (*p != 0) {
			Token token(Token::EMPTY);

			// Numbers
			if (*p >= '0' && *p <= '9') {
				char *out;
				token = Token(Token::NUMBER, strtof(p, &out));
				p = out;
				binary = true;
			// Identifiers
			} else if ( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') ) {
				const char *identifier = p;
				while ( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p >= '0' && *p <= '9'))
					p++;
				token = env.token_for_identifier(identifier, u32(p-identifier));
				binary = true;
			// Operators
			} else {
				switch (*p) {
					case '(': token = Token(Token::LEFT_PARENTHESIS); binary = false; break;
					case ')': token = Token(Token::RIGHT_PARENTHESIS); binary = true; break;
					case ' ': case '\t': case '\n': case '\r': break;
					case '-': token = env.token_for_identifier(binary ? "-" : "u-"); binary = false; break;
					case '+': token = env.token_for_identifier(binary ? "+" : "u+"); binary = false; break;

					default: {
						char s1[2] = {*p,0};
						char s2[3] = {*p, *(p+1), 0};

						if (s2[1] && env.has_function(s2)) {
							token = env.token_for_identifier(s2);
							++p;
						} else
							token = env.token_for_identifier(s1);

						binary = false;
						break;
					}
				}
				++p;
			}

			if (token.type != Token::EMPTY) {
				if (num_tokens == capacity)
					++overflow_tokens;
				else
					tokens[num_tokens++] = token;
			}
		}

		return num_tokens + overflow_tokens;
	}

	/// Performs constant folding on the program represented by @a rpl which is a
	/// sequence of tokens in reverse polish notation. Any function found in the
	/// token stream which only takes constant arguments is replaced by the
	/// result of evaluating the function over the constant arguments.
	static void fold_constants(Token *rpl, unsigned &num_tokens, CompileEnvironment &env)
	{
		static const int MAX_ARITY = 4;
		float stack_data[MAX_ARITY];

		for (unsigned i=0; i<num_tokens; ++i) {
			if (rpl[i].type != Token::FUNCTION)
				continue;

			Stack stack(stack_data, MAX_ARITY);
			bool constant_arguments = true;
			Function f = env.function_values[rpl[i].id];
			unsigned arity = f.arity;
			CE_ASSERT(arity <= MAX_ARITY, "MAX_ARITY too small");
			CE_ASSERT(i >= arity, "Too few arguments to function");
			unsigned arg_start = i - arity;
			for (unsigned j=0; j<arity && constant_arguments; ++j) {
				constant_arguments = constant_arguments && rpl[i-j-1].type == Token::NUMBER;
				stack.data[j] = rpl[arg_start+j].value;
			}
			if (!constant_arguments)
				continue;

			stack.size = arity;
			compute_function(f.op_code, stack);
			unsigned results = stack.size;
			int to_remove = int(arity + 1) - int(results);
			if (to_remove > 0) {
				memmove(&rpl[arg_start], &rpl[arg_start+to_remove], sizeof(Token)*(num_tokens-arg_start-to_remove));
				num_tokens -= to_remove;
			}
			for (unsigned res = 0; res<stack.size; ++res)
				rpl[arg_start + res] = Token(Token::NUMBER, stack.data[res]);
			i = arg_start - 1;
		}
	}

	/// Generates bytecode from a program in RPL token stream form.
	/// Returns the number of byte_code tokens generated. If the returned number is > capacity, only the first
	/// capacity items are generated.
	static unsigned generate_bytecode(Token *rpl, unsigned num_tokens, const CompileEnvironment &env, unsigned *byte_code, unsigned capacity)
	{
		unsigned size = 0;
		unsigned overflow = 0;

		for (unsigned i=0; i<num_tokens; ++i) {
			Function f;
			Token t = rpl[i];
			unsigned op;
			switch (t.type) {
				case Token::NUMBER:
					op = float_to_unsigned(t.value);
					break;
				case Token::VARIABLE:
					op = BC_PUSH_VAR + t.id;
					break;
				case Token::FUNCTION:
					f = env.function_values[t.id];
					op = BC_FUNCTION + f.op_code;
					break;
				default:
					op = 0xdeadbeef;
					CE_FATAL("Unknown token");
					break;
			}
			if (size < capacity)
				byte_code[size++] = op;
			else
				++overflow;
		}

		unsigned op = BC_END;
		if (size < capacity)
			byte_code[size++] = op;
		else
			++overflow;

		return size + overflow;
	}

	/// Represents an item on the function call stack.
	/// This object is comparable so that functions that have higher precedence are executed
	/// before others.
	struct FunctionStackItem
	{
		FunctionStackItem() {}
		FunctionStackItem(Token t, int p, int pl) : token(t), precedence(p), par_level(pl) {}
		inline int cmp(const FunctionStackItem &f) const {
			if (par_level != f.par_level) return par_level - f.par_level;
			return precedence - f.precedence;
		}
		inline bool operator<(const FunctionStackItem &other) const {return cmp(other) < 0;}
		inline bool operator<=(const FunctionStackItem &other) const {return cmp(other) <= 0;}
		inline bool operator==(const FunctionStackItem &other) const {return cmp(other) == 0;}
		inline bool operator>=(const FunctionStackItem &other) const {return cmp(other) >= 0;}
		inline bool operator>(const FunctionStackItem &other) const {return cmp(other) > 0;}
		Token token;
		int precedence;
		int par_level;
	};

	const int NUM_DEFAULT_FUNCTIONS = 12;

	/// Sets up the functions that should be usable in the language.
	static unsigned setup_functions(const char **names, Function *functions, unsigned capacity)
	{
		CE_ASSERT(capacity >= NUM_DEFAULT_FUNCTIONS, "Not enough space for default functions");
		CE_UNUSED(capacity);
		names[0] = ","; functions[0] = Function(OP_NOP, 1, 0);
		names[1] = "+"; functions[1] = Function(OP_ADD, 12, 2);
		names[2] = "-"; functions[2] = Function(OP_SUB, 12, 2);
		names[3] = "*"; functions[3] = Function(OP_MUL, 13, 2);
		names[4] = "/"; functions[4] = Function(OP_DIV, 13, 2);
		names[5] = "u-"; functions[5] = Function(OP_UNARY_MINUS, 16, 1);
		names[6] = "u+"; functions[6] = Function(OP_NOP, 16, 0);
		names[7] = "sin"; functions[7] = Function(OP_SIN, 17, 1);
		names[8] = "cos"; functions[8] = Function(OP_COS, 17, 1);
		names[9] = "abs"; functions[9] = Function(OP_ABS, 17, 1);
		names[10] = "match"; functions[10] = Function(OP_MATCH, 17, 2);
		names[11] = "match_2d"; functions[11] = Function(OP_MATCH_2D, 17, 4);
		return NUM_DEFAULT_FUNCTIONS;
	}

	unsigned compile(const char *source, unsigned num_variables, const char **variables,
				 unsigned num_constants, const char **constant_names, const float *constant_values,
				 unsigned *byte_code, unsigned capacity)
	{
		const char *function_names[NUM_DEFAULT_FUNCTIONS];
		Function functions[NUM_DEFAULT_FUNCTIONS];
		unsigned num_functions = setup_functions(function_names, functions, NUM_DEFAULT_FUNCTIONS);

		CompileEnvironment env;
		env.num_variables = num_variables;
		env.variable_names = variables;
		env.num_constants = num_constants;
		env.constant_names = constant_names;
		env.constant_values = constant_values;
		env.num_functions = num_functions;
		env.function_names = function_names;
		env.function_values = functions;

		unsigned num_tokens = tokenize(source, env, NULL, 0);

		// Change alloca to some other temp memory allocator if you want to
		Token *tokens = (Token *)alloca(sizeof(Token) * num_tokens);
		tokenize(source, env, tokens, num_tokens);

		Token *rpl = (Token *)alloca(sizeof(Token) * num_tokens);
		unsigned num_rpl = 0;

		FunctionStackItem *function_stack = (FunctionStackItem *)alloca(sizeof(FunctionStackItem)*num_tokens);
		unsigned num_function_stack = 0;

		int par_level = 0;
		for (unsigned i=0; i<num_tokens; ++i) {
			Token &token = tokens[i];
			switch (token.type) {
				case Token::NUMBER:
				case Token::VARIABLE:
					rpl[num_rpl++] = token;
					break;
				case Token::LEFT_PARENTHESIS:
					++par_level;
					break;
				case Token::RIGHT_PARENTHESIS:
					--par_level;
					break;
				case Token::FUNCTION: {
					FunctionStackItem f(token, env.function_values[token.id].precedence, par_level);
					while (num_function_stack>0 && function_stack[num_function_stack-1] >= f)
						rpl[num_rpl++] = function_stack[--num_function_stack].token;
					function_stack[num_function_stack++] = f;
					break;
				}
				default:
					CE_FATAL("Unknown token");
					break;
			}
		}

		while (num_function_stack>0)
			rpl[num_rpl++] = function_stack[--num_function_stack].token;

		fold_constants(rpl, num_rpl, env);
		return generate_bytecode(rpl, num_rpl, env, byte_code, capacity);
	}

	#endif // CAN_COMPILE

	bool run(const unsigned *byte_code, const float *variables, Stack &stack)
	{
		const unsigned *p = byte_code;
		while (true) {
			unsigned bc = *p++;
			unsigned op = bc_mask(bc);
			unsigned id = id_mask(bc);
			switch (op) {
				case BC_PUSH_VAR:
					if (stack.size == stack.capacity) return false;
					stack.data[stack.size++] = variables[id];
					break;
				case BC_FUNCTION:
					compute_function((OpCode)id, stack);
					break;
				case BC_END:
					return true;
				default: // BC_PUSH_FLOAT
					if (stack.size == stack.capacity) return false;
					stack.data[stack.size++] = unsigned_to_float(bc);
					break;

			}
		}
	}

} // namespace expression_language

} // namespace skinny

} // namespace crown
