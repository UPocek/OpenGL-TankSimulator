#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float oppacity;

void main()
{
    FragColor = vec4(color.r,color.g, color.b, oppacity);
}