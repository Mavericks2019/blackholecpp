#version 430 core
layout(location = 0) in vec2 position;
void main() {
    // 直接传递位置
    gl_Position = vec4(position, 0.0, 1.0);
}