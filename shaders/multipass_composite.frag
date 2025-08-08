#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D circleTexture;  // 圆形纹理（第一通道渲染的结果）
uniform vec3 rectColor;           // 矩形颜色
uniform vec2 rectCenter;          // 矩形中心（归一化坐标）
uniform vec2 rectSize;            // 矩形大小（归一化坐标）

void main() {
    // 计算矩形区域（归一化坐标）
    vec2 rectHalf = rectSize * 0.5;
    vec2 minBounds = rectCenter - rectHalf;
    vec2 maxBounds = rectCenter + rectHalf;
    
    // 修复：使用step函数替代布尔运算
    float inRectX = step(minBounds.x, TexCoord.x) - step(maxBounds.x, TexCoord.x);
    float inRectY = step(minBounds.y, TexCoord.y) - step(maxBounds.y, TexCoord.y);
    float inRect = inRectX * inRectY;
    
    vec4 rect = vec4(rectColor, inRect);
    
    // 从纹理中获取圆形颜色
    vec4 circle = texture(circleTexture, TexCoord);
    
    // 合成：使用圆形的alpha值进行混合
    FragColor = mix(rect, circle, circle.a);
}