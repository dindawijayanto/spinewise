#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aRegionId;

out vec3 vFragPos;
out vec3 vNormal;
flat out int vRegionId;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vRegionId = int(aRegionId + 0.5);

    gl_Position = uProjection * uView * worldPos;
}