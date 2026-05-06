include = [
	"core/shaders/common.shader"
]

render_states = {
	fallback = {
		states = {
			depth_enable = true
			depth_write_enable = true
			blend_enable = false
		}
	}
}

bgfx_shaders = {
	fallback = {
		includes = [ "common" ]

		varying = """
			vec3 a_position : POSITION;
		"""

		vs_input_output = """
			$input a_position
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			}
		"""

		fs_code = """
			void main()
			{
				gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			}
		"""
	}
}

shaders = {
	fallback = {
		bgfx_shader = "fallback"
		render_state = "fallback"
	}
}

static_compile = [
	{ shader = "fallback" defines = [] }
]
