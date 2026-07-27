#version 430 core
in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec2 iResolution;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform vec3 iChannelResolution[4];

vec3 ColorFetch(vec2 coord) {
    return texture(iChannel0, coord).rgb;
}

float weights[5];
float offsets[5];

void main() {
    weights[0] = 0.19638062;
    weights[1] = 0.29675293;
    weights[2] = 0.09442139;
    weights[3] = 0.01037598;
    weights[4] = 0.00025940;

    offsets[0] = 0.00000000;
    offsets[1] = 1.41176471;
    offsets[2] = 3.29411765;
    offsets[3] = 5.17647059;
    offsets[4] = 7.05882353;

    vec2 uv = texCoord;
    vec3 color = vec3(0.0);
    float weightSum = 0.0;

    if (uv.x < 0.52) {
        color += ColorFetch(uv) * weights[0];
        weightSum += weights[0];
        for(int i = 1; i < 5; i++) {
            vec2 offset = vec2(offsets[i]) / iResolution.xy;
            color += ColorFetch(uv + offset * vec2(0.5, 0.0)) * weights[i];
            color += ColorFetch(uv - offset * vec2(0.5, 0.0)) * weights[i];
            weightSum += weights[i] * 2.0;
        }
        color /= weightSum;
    }
    fragColor = vec4(color, 1.0);
}
