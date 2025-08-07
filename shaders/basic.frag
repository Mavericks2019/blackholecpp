#version 430 core
in vec2 fragCoord;
out vec4 outColor;

uniform float iTime;
uniform vec2 iResolution;

#define iterations 17
#define formuparam 0.53

#define volsteps 20
#define stepsize 0.1

#define zoom   0.800
#define tile   0.850
#define speed  0.002 

#define brightness 0.002
#define darkmatter 0.300
#define distfading 0.750
#define saturation 0.750

float SCurve (float value) {
    if (value < 0.5) {
        return value * value * value * value * value * 16.0; 
    }
    value -= 1.0;
    return value * value * value * value * value * 16.0 + 1.0;
}

void main() {
    // 获取坐标和方向
    vec2 uv = fragCoord - 0.5;
    uv.y *= iResolution.y / iResolution.x;
    vec3 dir = vec3(uv * zoom, 1.0);
    float time = iTime * speed + 0.25;

    // 自动旋转参数 - 完全基于时间
    float autoRotation = time * 0.5; // 自动旋转速度
    float a1 = 0.5 + autoRotation;
    float a2 = 0.8 + autoRotation * 0.7;
    
    // 创建旋转矩阵
    mat2 rot1 = mat2(cos(a1), sin(a1), -sin(a1), cos(a1));
    mat2 rot2 = mat2(cos(a2), sin(a2), -sin(a2), cos(a2));
    
    // 应用旋转
    dir.xz *= rot1;
    dir.xy *= rot2;
    
    vec3 from = vec3(1.0, 0.5, 0.5);
    from += vec3(time * 2.0, time, -2.0);
    from.xz *= rot1;
    from.xy *= rot2;
    
    // 体积渲染
    float s = 0.1, fade = 1.0;
    vec3 v = vec3(0.0);
    
    for (int r = 0; r < volsteps; r++) {
        vec3 p = from + s * dir * 0.5;
        p = abs(vec3(tile) - mod(p, vec3(tile * 2.0))); // 空间平铺折叠
        float pa, a = pa = 0.0;
        
        for (int i = 0; i < iterations; i++) { 
            p = abs(p) / dot(p, p) - formuparam; // 核心分形公式
            a += abs(length(p) - pa); // 距离变化量
            pa = length(p);
        }
        
        float dm = max(0.0, darkmatter - a * a * 0.001); // 暗物质计算
        a = pow(a, 2.5); // 增加对比度
        
        if (r > 6) fade *= 1.0 - dm; // 距离衰减
        
        v += fade;
        v += vec3(s, s*s, s*s*s*s) * a * brightness * fade; // 基于距离的颜色
        fade *= distfading; // 距离衰减
        s += stepsize;
    }
    
    v = mix(vec3(length(v)), v, saturation); // 饱和度调整
    
    vec4 C = vec4(v * 0.01, 1.0);
    C.r = pow(C.r, 0.35); 
    C.g = pow(C.g, 0.36); 
    C.b = pow(C.b, 0.4); 
    
    vec4 L = C;   
    C.r = mix(L.r, SCurve(C.r), 1.0); 
    C.g = mix(L.g, SCurve(C.g), 0.9); 
    C.b = mix(L.b, SCurve(C.b), 0.6);     
    
    outColor = C;
}