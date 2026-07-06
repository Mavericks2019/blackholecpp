#version 430 core
layout(location = 0) in vec2 position;
out vec2 texCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    // 将位置从[-1,1]转换到[0,1]纹理坐标
    texCoord = position * 0.5 + 0.5;
}