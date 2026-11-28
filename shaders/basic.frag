#version 330 core

in vec3 vColor;
in vec3 vLocalPos;
out vec4 FragColor;

uniform float uAlpha;
uniform int uEnableSideViz;
uniform float uConvexDir; // -1.0 for Right (-X convex), +1.0 for Left (+X convex)
uniform vec3 uConvexColor;
uniform vec3 uConcaveColor;

void main()
{
    vec3 finalColor = vColor;
    if (uEnableSideViz != 0) {
        // vLocalPos.x is in range [-0.5, 0.5]
        // If uConvexDir == -1.0 (Right convex), then negative local x is convex, positive is concave.
        float sideFactor = uConvexDir * vLocalPos.x;
        float blendT = clamp(sideFactor * 2.0 + 0.5, 0.0, 1.0);
        vec3 sideColor = mix(uConcaveColor, uConvexColor, blendT);
        finalColor = mix(finalColor, sideColor, 0.45);
    }
    FragColor = vec4(finalColor, uAlpha);
}
