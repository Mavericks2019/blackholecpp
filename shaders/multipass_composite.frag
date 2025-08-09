#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 TexCoord;
uniform sampler2D circleTexture;  // 现在包含分形效果的纹理
uniform vec3 rectColor;
uniform vec2 rectCenter;
uniform vec2 rectSize;

void main() {
    // 计算矩形边界
    vec2 rectHalf = rectSize * 0.5;
    bool inRect = abs(TexCoord.x - rectCenter.x) < rectHalf.x && 
                  abs(TexCoord.y - rectCenter.y) < rectHalf.y;
    
    // 从纹理中获取分形颜色
    vec4 fractalColor = texture2D(circleTexture, TexCoord);
    
    // 在矩形区域内显示矩形颜色，否则显示分形效果
    if (inRect) {
        // 选项1：纯色矩形
        gl_FragColor = vec4(rectColor, 1.0);
        
        // 选项2：混合矩形和分形效果（取消注释启用）
        // float blendFactor = 0.7; // 混合比例 (0.0-1.0)
        // gl_FragColor = mix(fractalColor, vec4(rectColor, 1.0), blendFactor);
    } else {
        gl_FragColor = fractalColor;
    }
}