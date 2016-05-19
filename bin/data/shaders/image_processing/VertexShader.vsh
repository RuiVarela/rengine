//!pragma semantic texture_coords texcoords
//!pragma semantic position position
uniform mat4 mvp;

in vec3 position;
in vec2 texture_coords;
void main()
{			
	texture_coordinate = texture_coords;
	gl_Position = mvp * vec4(position, 1.0);
}
