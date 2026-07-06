#version 430 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 fragCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    fragCoord = texCoord * vec2(1.0, 1.0);
}