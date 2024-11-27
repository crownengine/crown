#include "resource/expression_language.h"

namespace crown
{
namespace expression_language
{
	inline float pop(Stack &stack)
	{
		CE_ASSERT(stack.size > 0, "Stack underflow");
		return stack.data[--stack.size];
	}

	inline void push(Stack &stack, float f)
	{
		CE_ASSERT(stack.size < stack.capacity, "Stack overflow");
		stack.data[stack.size++] = f;
	}

} // namespace expression_language

} // namespace crown
