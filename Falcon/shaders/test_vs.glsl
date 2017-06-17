#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 mat_model; // our matrix
uniform mat4 mat_view; // our matrix
uniform mat4 mat_projection; // our matrix

out vec3 position_eye, normal_eye;

void main() {
   	position_eye = vec3 (mat_view * mat_model * vec4 (vertex_position, 1.0));
    normal_eye = vec3 (mat_view * mat_model * vec4 (vertex_normal, 0.0));
    gl_Position = mat_projection * vec4 (position_eye, 1.0);
}
