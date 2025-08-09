#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
uniform vec2 center;
uniform float radius;
uniform vec3 circleColor;
void main() {
    float d = distance(gl_FragCoord.xy, center);
    float alpha = smoothstep(radius, radius - 1.0, d);
    gl_FragColor = vec4(circleColor, alpha);
}