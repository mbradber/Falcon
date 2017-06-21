#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in int bone_id;

uniform mat4 mat_model, mat_view, mat_projection;
uniform mat4 bone_matrices[32];

out vec3 normal;
out vec2 st;
out vec3 color;

void main() {
   	color = vec3 (0.0, 0.0, 0.0);
    if (bone_id == 0) {
        color.r = 1.0;
    } else if (bone_id == 1) {
        color.g = 1.0;
    } else if (bone_id == 2) {
        color.b = 1.0;
    } 
    
    st = texture_coord;
    normal = vertex_normal;
    gl_Position = mat_projection * mat_view * mat_model * bone_matrices[bone_id] * vec4 (vertex_position, 1.0);
}
