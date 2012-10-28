#ifndef __GLOBALS_H__
#define __GLOBALS_H__

class Joystick;

enum CollisionGroups
{
	CG_NONE = 0,
	CG_ZOMBIE = 1,
	CG_PG = 2,
	CG_BULLET = 3,
	CG_WALL = 4
};

class Globals
{
public:
	static Joystick* joystick;

private:
	Globals() {};
};

#endif //__GLOBALS_H__