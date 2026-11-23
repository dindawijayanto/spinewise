#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
flat in int vRegionId;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec3 uLightPos;

uniform int uHoveredRegion;
uniform int uSelectedRegion;

void main()
{
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vFragPos);
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // Warna dasar tulang
    vec3 baseColor = vec3(0.84, 0.81, 0.75);

    if (vRegionId == 1)
        baseColor = vec3(0.18, 0.80, 0.92); // Cervical (Cyan)
    else if (vRegionId == 2)
        baseColor = vec3(0.22, 0.84, 0.58); // Thoracic (Hijau)
    else if (vRegionId == 3)
        baseColor = vec3(0.95, 0.74, 0.22); // Lumbar (Kuning/Emas)

    // Highlight tipis saat hover saja; saat diklik/selected warna regio tetap asli
    if (vRegionId != 0 && vRegionId == uHoveredRegion && uHoveredRegion != uSelectedRegion)
    {
        baseColor = mix(baseColor, vec3(1.0, 1.0, 1.0), 0.20);
    }

    // Blinn-Phong lighting
    float ambient = 0.28;
    float diff = max(dot(norm, lightDir), 0.0);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0) * 0.25;

    vec3 result = (ambient + diff * 0.75) * baseColor + vec3(spec);
    FragColor = vec4(result, 1.0);
}