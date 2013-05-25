in vec4 vertex;
in vec2 coords;

// Model-View-Projection matrix
uniform mat4 mvp_matrix;
uniform vec3 color;

void main(void)
{
	gl_Position = mvp_matrix * vertex;

	gl_FrontColor = vec4(color, 1.0);
}
