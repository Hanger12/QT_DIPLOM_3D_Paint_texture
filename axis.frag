const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;

in vec3 v_color;

void main()
{
    oFragColor = vec4(v_color, 1.0);
}
