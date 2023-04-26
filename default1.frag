#version 330 core

const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;
layout(location = 1) out vec4 oUv;

in vec2 v_texCoord;
in vec4 v_position;
in vec3 v_normal;

uniform vec3 u_lightposition;
uniform bool u_linemode;
uniform float u_lightpower;
void main()
{
    if (u_linemode)
    {
        oFragColor = vec4(0.0, 1.0, 0.0, 1.0);
        oUv = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        vec4 resultcolor = vec4(0.0,0.0,0.0,0.0);
        vec4 diffmatcolor = vec4(0.918,0.918,0.949,1.0);
        vec4 eyeposition = vec4(0.0,0.0,0.0,1.0);

        vec3 eyeVect = normalize(v_position.xyz-eyeposition.xyz);
        vec3 lightVect = normalize(v_position.xyz-u_lightposition);
        vec3 reflectlight = normalize(reflect(lightVect,v_normal));
        float specularfactor = 50.0;
        float ambientfactor = 0.1;
        float len= length(v_position.xyz-eyeposition.xyz);
        vec4 diffcolor = diffmatcolor * u_lightpower * max(0.0,dot(v_normal,-lightVect))/(1.0*0.25*pow(len,2));
        resultcolor +=diffcolor;
        vec4 ambientcolor = ambientfactor * diffmatcolor;
        resultcolor += ambientcolor;
        oFragColor = resultcolor;
        oUv = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
