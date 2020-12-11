#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;

out vec3 bColor;

void main()
{
    bColor = aColor;
    gl_Position = projection*view*transform*vec4(aPos.x, aPos.y, 1.0, 1.0);
}

