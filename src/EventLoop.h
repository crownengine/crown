#pragma once

namespace crown
{

/**
	Manages OS events. (Keyboard, mouse ecc.)
*/
class EventLoop
{
public:

	void	consume_events();
};

} // namespace crown

