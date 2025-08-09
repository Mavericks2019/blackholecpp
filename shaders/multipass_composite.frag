#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 TexCoord;
uniform sampler2D circleTexture;
uniform vec3 rectColor;
uniform vec2 rectCenter;
uniform vec2 rectSize;
void main() {
    vec2 rectHalf = rectSize * 0.5;
    bool inRect = abs(TexCoord.x - rectCenter.x) < rectHalf.x && 
                  abs(TexCoord.y - rectCenter.y) < rectHalf.y;
    vec4 rect = vec4(0.0);
    if (inRect) {
        rect = vec4(rectColor, 1.0);
    }
    vec4 circle = texture2D(circleTexture, TexCoord);
    gl_FragColor = mix(rect, circle, circle.a);
}