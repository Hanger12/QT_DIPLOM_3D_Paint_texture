#version 330 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;
out vec4 v_position;
out vec3 v_normal;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
void main()
{
    mat4 mv_matrix=u_viewMatrix*u_modelMatrix;
    gl_Position = u_projectionMatrix * mv_matrix * a_position;
    v_texCoord = a_texCoord;
    v_normal = normalize(vec3(mv_matrix * vec4(a_normal, 0.0)));
    v_position = mv_matrix*a_position;
}
