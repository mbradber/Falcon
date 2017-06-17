#version 410

in vec3 position_eye, normal_eye;

uniform mat4 view_mat;

// fixed point light properties
vec3 light_position_world  = vec3 (0.0, 0.0, 2.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour

// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 100.0; // specular 'power'

out vec4 fragment_colour; // final colour of surface

void main() {
    // ambient intensity
    vec3 Ia = La * Ka;
    
    // diffuse intensity
    vec3 Id = vec3 (0.0, 0.0, 0.0); // replace me later
    
    // specular intensity
    vec3 Is = vec3 (0.0, 0.0, 0.0); // replace me later //
    
    fragment_colour = vec4 (Is + Id + Ia, 1.0);
}
