#version 330 core 

layout(location = 0) in vec2 inPos;

uniform vec2 value;

void main()
{
	if(inPos.y < 0){
		gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
	}else{
		gl_Position = vec4(value.x, value.y, 0.0, 1.0);
	}

}