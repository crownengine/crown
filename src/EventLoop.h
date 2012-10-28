#pragma once

namespace Crown
{

/**
	Manages OS events. (Keyboard, mouse ecc.)
*/
class EventLoop
{
public:

	void	consume_events();
};

} // namespace Crown

