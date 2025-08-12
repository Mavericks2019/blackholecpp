#version 430 core
uniform sampler2D iChannel0;
uniform vec2 iResolution;
out vec4 fragColor;

vec3 ColorFetch(vec2 coord) {
    return texture(iChannel0, coord).rgb;   
}

void main() {
    float weights[5];
    float offsets[5];
    
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
    
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec3 color = vec3(0.0);
    float weightSum = 0.0;
    
    if (uv.x < 0.52) {
        color += ColorFetch(uv) * weights[0];
        weightSum += weights[0];

        for(int i = 1; i < 5; i++) {
            vec2 offset = vec2(offsets[i]) / iResolution.xy;
            color += ColorFetch(uv + offset * vec2(0.0, 0.5)) * weights[i];
            color += ColorFetch(uv - offset * vec2(0.0, 0.5)) * weights[i];
            weightSum += weights[i] * 2.0;
        }

        color /= weightSum;
    }

    fragColor = vec4(color, 1.0);
}