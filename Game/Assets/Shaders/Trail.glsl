--- vertex
layout(location = 0) in vec3 vertexPosition;


uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;


void main()
{
	gl_Position = proj*view*model*vec4(vertexPosition, 1.0);

}


--- fragment

uniform vec4 inputColor;

out vec4 outColor;

void main()
{	
	outColor = inputColor;
}

