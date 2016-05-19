uniform vec3 pin_position;

void main()
{
	vec4 vertex = gl_Vertex;
	vertex.x += pin_position.x;
	vertex.y += pin_position.y;
	vertex.z += pin_position.z;
	
	gl_Position = gl_ModelViewProjectionMatrix * (gl_Vertex + vec4(pin_position, 1.0));
}

