layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_color;

out vec3 v_color;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
void main()
{
    mat4 ModelViewProjectionMatrix = translate(mat4(), vec3(-0.8f, -0.8f, 0.0f))
            * u_projectionMatrix * translate(mat4(),vec3(0.0f, 0.0f, -1.0f))
            * mat4(mat3(u_viewMatrix))
            * scale(mat4(),vec3(0.05f));
    v_color = a_color;
}
