#version 330 core
out vec4 FragColor;

uniform vec2 center;       // 圆心位置（屏幕坐标）
uniform float radius;      // 圆半径
uniform vec3 circleColor;  // 圆颜色

void main() {
    // 计算当前片段到圆心的距离
    float d = distance(gl_FragCoord.xy, center); // 使用gl_FragCoord
    // 使用smoothstep实现边缘抗锯齿
    float alpha = 1.0 - smoothstep(radius - 1.0, radius + 1.0, d);
    FragColor = vec4(circleColor, alpha);
}