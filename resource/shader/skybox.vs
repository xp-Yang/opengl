#version 330 core
layout (location = 0) in vec3 vertex_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 pass_uv;

void main()
{
    gl_Position = projection * view * model * vec4(vertex_pos, 1.0);

    pass_uv = vertex_pos;
}