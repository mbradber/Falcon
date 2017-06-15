#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;

uniform mat4 mat_model; // our matrix
uniform mat4 mat_view; // our matrix
uniform mat4 mat_projection; // our matrix

out vec3 colour;

void main() {
    colour = vertex_colour;
    gl_Position = mat_projection * mat_view * mat_model * vec4(vertex_position, 1.0);
}
