#version 330 core

in vec3 chCol;
out vec4 outColUros;

uniform vec3 uLightColor;

void main()
{
 outColUros = vec4(uLightColor.r,uLightColor.g,uLightColor.b, 0.99);
}