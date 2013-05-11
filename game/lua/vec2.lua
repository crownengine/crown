local ffi = require("ffi")

ffi.cdef
[[
	typedef struct 
	{
		float x;
		float y;
	}
	Vec2;

	Vec2&				vec2(float nx, float ny);					

	Vec2&				vec2_add(Vec2& self, const Vec2& a);			

	Vec2& 				vec2_subtract(Vec2& self, const Vec2& a);			

	Vec2&				vec2_multiply(Vec2& self, float k);			

	Vec2&				vec2_divide(Vec2& self, float k);

	float				vec2_dot(Vec2& self, const Vec2& a);				

	bool				vec2_equals(Vec2& self, const Vec2& other);	

	bool				vec2_lower(Vec2& self, const Vec2& other);		

	bool				vec2_greater(Vec2& self, const Vec2& other);

	float				vec2_length(Vec2& self);

	float				vec2_squared_length(Vec2& self);

	void				vec2_set_length(Vec2& self, float len);					

	Vec2&				vec2_normalize(Vec2& self);

	Vec2&				vec2_negate(Vec2& self);

	float				vec2_get_distance_to(Vec2& self, const Vec2& a);

	float				vec2_get_angle_between(Vec2& self, const Vec2& a);

	void				vec2_zero(Vec2& self);
]]

Vec2 = {}

Vec2.vec2  				= lib.vec2
Vec2.add				= lib.vec2_add			
Vec2.subtract			= lib.vec2_subtract			
Vec2.multiply			= lib.vec2_multiply
Vec2.divide				= lib.vec2_divide
Vec2.dot				= lib.vec2_dot				
Vec2.equals				= lib.vec2_equals	
Vec2.lower				= lib.vec2_lower		
Vec2.greater			= lib.vec2_greater
Vec2.length				= lib.vec2_length
Vec2.squared_length		= lib.vec2_squared_length
Vec2.set_length			= lib.vec2_set_length					
Vec2.normalize			= lib.vec2_normalize
Vec2.negate				= lib.vec2_negate
Vec2.get_distance_to	= lib.vec2_get_distance_to
Vec2.get_angle_between	= lib.vec2_get_angle_between
Vec2.zero				= lib.vec2_zero