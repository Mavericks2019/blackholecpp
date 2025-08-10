#version 430 core
out vec4 fragColor;
uniform vec3 circleColor;
uniform vec2 iResolution;  // 视口分辨率
uniform vec2 offset;       // 偏移参数
uniform float radius;      // 半径参数
uniform float MBlackHole;  // 黑洞质量（太阳质量单位）
uniform sampler2D backgroundTexture;  // 背景纹理
uniform int backgroundType; // 0: 棋盘, 1: 纯黑, 2: 星空, 3: 纹理
uniform vec4 iMouse; // 添加 iMouse 变量
uniform float iTime;              // 添加 iTime 变量 (类似Shadertoy)
uniform sampler2D iChannel1;         // 棋盘格纹理 (类似Shadertoy)
uniform sampler2D iChannel3;        // 上一帧纹理
uniform float iTimeDelta = 0.16;
uniform vec3 iChannelResolution;  // 声明为vec3数组
uniform int iFrame;           // 添加 iFrame 变量 (类似Shadertoy)

// 物理常量
#define PI 3.141592653589
#define G0 6.673e-11
#define lightspeed 299792458.0
#define sigma 5.670373e-8
#define ly 9460730472580800.0
#define Msun 1.9891e30
#define FOV 0.5

const float kPi              = 3.141592653589;
const float kGravityConstant = 6.673e-11;
const float kSpeedOfLight    = 299792458.0;
const float kSigma           = 5.670373e-8;
const float kLightYear       = 9460730472580800.0;
const float kSolarMass       = 1.9884e30;

float RandomStep(vec2 Input, float Seed)
{
    return fract(sin(dot(Input + fract(11.4514 * sin(Seed)), vec2(12.9898, 78.233))) * 43758.5453);
}

float CubicInterpolate(float x)
{
    return 3.0 * x * x - 2.0 * x * x * x;
}

float PerlinNoise(vec3 Position)
{
    vec3 PosInt   = floor(Position);
    vec3 PosFloat = fract(Position);

    float v000 = 2.0 * fract(sin(dot(vec3(PosInt.x,       PosInt.y,       PosInt.z),       vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v100 = 2.0 * fract(sin(dot(vec3(PosInt.x + 1.0, PosInt.y,       PosInt.z),       vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v010 = 2.0 * fract(sin(dot(vec3(PosInt.x,       PosInt.y + 1.0, PosInt.z),       vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v110 = 2.0 * fract(sin(dot(vec3(PosInt.x + 1.0, PosInt.y + 1.0, PosInt.z),       vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v001 = 2.0 * fract(sin(dot(vec3(PosInt.x,       PosInt.y,       PosInt.z + 1.0), vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v101 = 2.0 * fract(sin(dot(vec3(PosInt.x + 1.0, PosInt.y,       PosInt.z + 1.0), vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v011 = 2.0 * fract(sin(dot(vec3(PosInt.x,       PosInt.y + 1.0, PosInt.z + 1.0), vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;
    float v111 = 2.0 * fract(sin(dot(vec3(PosInt.x + 1.0, PosInt.y + 1.0, PosInt.z + 1.0), vec3(12.9898, 78.233, 213.765))) * 43758.5453) - 1.0;

    float v00 = v001 * CubicInterpolate(PosFloat.z) + v000 * CubicInterpolate(1.0 - PosFloat.z);
    float v10 = v101 * CubicInterpolate(PosFloat.z) + v100 * CubicInterpolate(1.0 - PosFloat.z);
    float v01 = v011 * CubicInterpolate(PosFloat.z) + v010 * CubicInterpolate(1.0 - PosFloat.z);
    float v11 = v111 * CubicInterpolate(PosFloat.z) + v110 * CubicInterpolate(1.0 - PosFloat.z);
    float v0  = v01  * CubicInterpolate(PosFloat.y) + v00  * CubicInterpolate(1.0 - PosFloat.y);
    float v1  = v11  * CubicInterpolate(PosFloat.y) + v10  * CubicInterpolate(1.0 - PosFloat.y);

    return v1 * CubicInterpolate(PosFloat.x) + v0 * CubicInterpolate(1.0 - PosFloat.x);
}

float SoftSaturate(float x)
{
    return 1.0 - 1.0 / (max(x, 0.0) + 1.0);
}

float GenerateAccretionDiskNoise(vec3 Position, int NoiseStartLevel, int NoiseEndLevel, float ContrastLevel)
{
    float NoiseAccumulator = 10.0;
    float NoiseFrequency   = 1.0;
    
    for (int Level = NoiseStartLevel; Level < NoiseEndLevel; ++Level)
    {
        NoiseFrequency = pow(3.0, float(Level));
        vec3 ScaledPosition = vec3(NoiseFrequency * Position.x, NoiseFrequency * Position.y, NoiseFrequency * Position.z);

        NoiseAccumulator *= (1.0 + 0.1 * PerlinNoise(ScaledPosition));
    }
    
    return log(1.0 + pow(0.1 * NoiseAccumulator, ContrastLevel));
}

float Vec2ToTheta(vec2 v1, vec2 v2)
{
    if (dot(v1, v2) > 0.0)
    {
        return asin(0.999999 * (v1.x * v2.y - v1.y * v2.x) / length(v1) / length(v2));
    }
    else if (dot(v1, v2) < 0.0 && (-v1.x * v2.y + v1.y * v2.x) < 0.0)
    {
        return kPi - asin(0.999999 * (v1.x * v2.y - v1.y * v2.x) / length(v1) / length(v2));
    }
    else if (dot(v1, v2) < 0.0 && (-v1.x * v2.y + v1.y * v2.x) > 0.0)
    {
        return -kPi - asin(0.999999 * (v1.x * v2.y - v1.y * v2.x) / length(v1) / length(v2));
    }
}

vec3 KelvinToRgb(float Kelvin)
{
    if (Kelvin < 400.01)
    {
        return vec3(0.0);
    }

    float Teff     = (Kelvin - 6500.0) / (6500.0 * Kelvin * 2.2);
    vec3  RgbColor = vec3(0.0);
    
    RgbColor.r = exp(2.05539304e4 * Teff);
    RgbColor.g = exp(2.63463675e4 * Teff);
    RgbColor.b = exp(3.30145739e4 * Teff);

    float BrightnessScale = 1.0 / max(max(RgbColor.r, RgbColor.g), RgbColor.b);
    
    if (Kelvin < 1000.0)
    {
        BrightnessScale *= (Kelvin - 400.0) / 600.0;
    }
    
    RgbColor *= BrightnessScale;
    return RgbColor;
}

float GetKeplerianAngularVelocity(float Radius, float Rs)
{
    return sqrt(kSpeedOfLight / kLightYear * kSpeedOfLight * Rs / kLightYear / ((2.0 * Radius - 3.0 * Rs) * Radius * Radius));
}

vec3 WorldToBlackHoleSpace(vec4 Position, vec3 BlackHolePos, vec3 DiskNormal,vec3 WorldUp)
{
    if (DiskNormal == WorldUp)
    {
        DiskNormal += 0.0001 * vec3(1.0, 0.0, 0.0);
    }

    vec3 BlackHoleSpaceY = normalize(DiskNormal);
    vec3 BlackHoleSpaceZ = normalize(cross(WorldUp, BlackHoleSpaceY));
    vec3 BlackHoleSpaceX = normalize(cross(BlackHoleSpaceY, BlackHoleSpaceZ));

    mat4x4 Translate = mat4x4(1.0, 0.0, 0.0, -BlackHolePos.x,
                              0.0, 1.0, 0.0, -BlackHolePos.y,
                              0.0, 0.0, 1.0, -BlackHolePos.z,
                              0.0, 0.0, 0.0, 1.0);

    mat4x4 Rotate = mat4x4(BlackHoleSpaceX.x, BlackHoleSpaceX.y, BlackHoleSpaceX.z, 0.0,
                           BlackHoleSpaceY.x, BlackHoleSpaceY.y, BlackHoleSpaceY.z, 0.0,
                           BlackHoleSpaceZ.x, BlackHoleSpaceZ.y, BlackHoleSpaceZ.z, 0.0,
                           0.0,               0.0,               0.0,               1.0);

    Position = transpose(Translate) * Position;
    Position = transpose(Rotate)    * Position;
    return Position.xyz;
}

vec3 ApplyBlackHoleRotation(vec4 Position, vec3 BlackHolePos, vec3 DiskNormal,vec3 WorldUp)
{
    if (DiskNormal == WorldUp)
    {
        DiskNormal += 0.0001 * vec3(1.0, 0.0, 0.0);
    }

    vec3 BlackHoleSpaceY = normalize(DiskNormal);
    vec3 BlackHoleSpaceZ = normalize(cross(WorldUp, BlackHoleSpaceY));
    vec3 BlackHoleSpaceX = normalize(cross(BlackHoleSpaceY, BlackHoleSpaceZ));

    mat4x4 Rotate = mat4x4(BlackHoleSpaceX.x, BlackHoleSpaceX.y, BlackHoleSpaceX.z, 0.0,
                           BlackHoleSpaceY.x, BlackHoleSpaceY.y, BlackHoleSpaceY.z, 0.0,
                           BlackHoleSpaceZ.x, BlackHoleSpaceZ.y, BlackHoleSpaceZ.z, 0.0,
                           0.0,               0.0,               0.0,               1.0);

    Position = transpose(Rotate) * Position;
    return Position.xyz;
}

vec4 GetCamera(vec4 Position)  // 相机系平移旋转  本部分在实际使用时uniform输入
{
    float Theta = 4.0 * kPi * iMouse.x / iResolution.x;
    float Phi   = 0.999 * kPi * iMouse.y / iResolution.y + 0.0005;
    float R     = 0.000057;

    if (iFrame < 2)
    {
        Theta = 4.0 * kPi * 0.45;
        Phi   = 0.999 * kPi * 0.55 + 0.0005;
    }
    // if (texelFetch(iChannel1, ivec2(83, 0), 0).x > 0.)
    // {
    //     R = 0.000097;
    // }
    // if (texelFetch(, ivec2(87, 0), 0).x > 0.)
    // {
    //     R = 0.0000186;
    // }
    vec3 Rotcen = vec3(0.0, 0.0, 0.0);

    vec3 Campos;

    vec3 reposcam = vec3(R * sin(Phi) * cos(Theta), -R * cos(Phi), -R * sin(Phi) * sin(Theta));

    Campos    = Rotcen + reposcam;
    vec3 vecy = vec3(0.0, 1.0, 0.0);

    vec3 X = normalize(cross(vecy, reposcam));
    vec3 Y = normalize(cross(reposcam, X));
    vec3 Z = normalize(reposcam);

    Position = (transpose(mat4x4(1., 0., 0., -Campos.x, 0., 1., 0., -Campos.y, 0., 0., 1., -Campos.z, 0., 0., 0., 1.)) * Position);

    Position = transpose(mat4x4(X.x, X.y, X.z, 0., Y.x, Y.y, Y.z, 0., Z.x, Z.y, Z.z, 0., 0., 0., 0., 1.)) * Position;

    return Position;
}

vec4 GetCameraRot(vec4 Position)  // 摄影机系旋转    本部分在实际使用时uniform输入
{
    float Theta = 4.0 * kPi * iMouse.x / iResolution.x;
    float Phi   = 0.999 * kPi * iMouse.y / iResolution.y + 0.0005;
    float R     = 0.000057;

    if (iFrame < 2)
    {
        Theta = 4.0 * kPi * 0.45;
        Phi   = 0.999 * kPi * 0.55 + 0.0005;
    }
    // if (texelFetch(iChannel0, ivec2(83, 0), 0).x > 0.)
    // {
    //     R = 0.000097;
    // }
    // if (texelFetch(iChannel0, ivec2(87, 0), 0).x > 0.)
    // {
    //     R = 0.0000186;
    // }
    vec3 Rotcen = vec3(0.0, 0.0, 0.0);

    vec3 Campos;

    vec3 reposcam = vec3(R * sin(Phi) * cos(Theta), -R * cos(Phi), -R * sin(Phi) * sin(Theta));

    Campos    = Rotcen + reposcam;
    vec3 vecy = vec3(0.0, 1.0, 0.0);

    vec3 X = normalize(cross(vecy, reposcam));
    vec3 Y = normalize(cross(reposcam, X));
    vec3 Z = normalize(reposcam);
    
    Position = transpose(mat4x4(X.x, X.y, X.z, 0., Y.x, Y.y, Y.z, 0., Z.x, Z.y, Z.z, 0., 0., 0., 0., 1.)) * Position;
    return Position;
}

vec3 FragUvToDir(vec2 FragUv, float Fov)
{
    return normalize(vec3(Fov * (2.0 * FragUv.x - 1.0), Fov * (2.0 * FragUv.y - 1.0) * iResolution.y / iResolution.x, -1.0));
}

vec2 PosToNdc(vec4 Pos)
{
    return vec2(-Pos.x / Pos.z, -Pos.y / Pos.z * iResolution.x / iResolution.y);
}

vec2 DirToNdc(vec3 Dir)
{
    return vec2(-Dir.x / Dir.z, -Dir.y / Dir.z * iResolution.x / iResolution.y);
}

vec2 DirToFragUv(vec3 Dir)
{
    return vec2(0.5 - 0.5 * Dir.x / Dir.z, 0.5 - 0.5 * Dir.y / Dir.z * iResolution.x / iResolution.y);
}

vec2 PosToFragUv(vec4 Pos)
{
    return vec2(0.5 - 0.5 * Pos.x / Pos.z, 0.5 - 0.5 * Pos.y / Pos.z * iResolution.x / iResolution.y);
}

float Shape(float x, float Alpha, float Beta)
{
    float k = pow(Alpha + Beta, Alpha + Beta) / (pow(Alpha, Alpha) * pow(Beta, Beta));
    return k * pow(x, Alpha) * pow(1.0 - x, Beta);
}

vec4 DiskColor(vec4 BaseColor, float TimeRate, float StepLength, vec3 RayPos, vec3 LastRayPos,
               vec3 RayDir, vec3 LastRayDir, vec3 WorldUp, vec3 BlackHolePos, vec3 DiskNormal,
               float Rs, float InterRadius, float OuterRadius, float DiskTemperatureArgument,
               float QuadraticedPeakTemperature, float ShiftMax)
{
    vec3 CameraPos = WorldToBlackHoleSpace(vec4(0.0, 0.0, 0.0, 1.0), BlackHolePos, DiskNormal, WorldUp);
    vec3 PosOnDisk = WorldToBlackHoleSpace(vec4(RayPos, 1.0),        BlackHolePos, DiskNormal, WorldUp);
    vec3 DirOnDisk = ApplyBlackHoleRotation(vec4(RayDir, 1.0),       BlackHolePos, DiskNormal, WorldUp);

    float PosR = length(PosOnDisk.zx);
    float PosY = PosOnDisk.y;

    vec4 Color = vec4(0.0);
    if (abs(PosY) < 0.5 * Rs && PosR < OuterRadius && PosR > InterRadius)
    {
        float EffectiveRadius = 1.0 - ((PosR - InterRadius) / (OuterRadius - InterRadius) * 0.5);
        if ((OuterRadius - InterRadius) > 9.0 * Rs)
        {
            if (PosR < 5.0 * Rs + InterRadius)
            {
                EffectiveRadius = 1.0 - ((PosR - InterRadius) / (9.0 * Rs) * 0.5);
            }
            else
            {
                EffectiveRadius = 1.0 - (0.5 / 0.9 * 0.5 + ((PosR - InterRadius) / (OuterRadius - InterRadius) -
                                  5.0 * Rs / (OuterRadius - InterRadius)) / (1.0 - 5.0 * Rs / (OuterRadius - InterRadius)) * 0.5);
            }
        }

        if ((abs(PosY) < 0.5 * Rs * Shape(EffectiveRadius, 4.0, 0.9)) || (PosY < 0.5 * Rs * (1.0 - 5.0 * pow(2.0 * (1.0 - EffectiveRadius), 2.0))))
        {
            float AngularVelocity  = GetKeplerianAngularVelocity(PosR, Rs);
            float HalfPiTimeInside = kPi / GetKeplerianAngularVelocity(3.0 * Rs, Rs);

            float SpiralTheta=12.0*2.0/sqrt(3.0)*(atan(sqrt(0.6666666*(PosR/Rs)-1.0)));
            float InnerTheta= kPi / HalfPiTimeInside *iTime * TimeRate ;
            float PosThetaForInnerCloud = Vec2ToTheta(PosOnDisk.zx, vec2(cos(0.666666*InnerTheta),sin(0.666666*InnerTheta)));
            float PosTheta            = Vec2ToTheta(PosOnDisk.zx, vec2(cos(-SpiralTheta), sin(-SpiralTheta)));

            // 计算盘温度
            float DiskTemperature = pow(DiskTemperatureArgument * pow(max(Rs/PosR,0.10),3.0) * max(1.0 - sqrt(InterRadius / PosR), 0.000001), 0.25);
            // 计算云相对速度
            vec3  CloudVelocity    = kLightYear / kSpeedOfLight * AngularVelocity * cross(vec3(0., 1., 0.), PosOnDisk);
            float RelativeVelocity = dot(-DirOnDisk, CloudVelocity);
            // 计算多普勒因子
            float Dopler = sqrt((1.0 + RelativeVelocity) / (1.0 - RelativeVelocity));
            // 总红移量，含多普勒因子和引力红移和
            float RedShift = Dopler * sqrt(max(1.0 - Rs / PosR, 0.000001)) / sqrt(max(1.0 - Rs / length(CameraPos), 0.000001));

            float Density           = 0.0;
            float Thick             = 0.0;
            float VerticalMixFactor = 0.0;
            float DustColor         = 0.0;
            
            float RotPosR=PosR/Rs+0.3*sqrt(3.0)*kSpeedOfLight/kLightYear /3.0/sqrt(3.0)/Rs*TimeRate*iTime;
            
            vec4  Color0            = vec4(0.0);
            
            Density = Shape(EffectiveRadius, 4.0, 0.9);
            if (abs(PosY) < 0.5 * Rs * Density)
            {
                Thick = 0.5 * Rs * Density * (0.4 + 0.6 * SoftSaturate(GenerateAccretionDiskNoise(vec3(1.5 * PosTheta,RotPosR, 1.0), 1, 3, 80.0))); // 盘厚
                VerticalMixFactor = max(0.0, (1.0 - abs(PosY) / Thick));
                Density    *= 0.7 * VerticalMixFactor * Density;
                Color0      = vec4(GenerateAccretionDiskNoise(vec3(1.0 * RotPosR, 1.0 * PosY / Rs, 0.5 * PosTheta), 3, 6, 80.0)); // 云本体
                Color0.xyz *= Density * 1.4 * (0.2 + 0.8 * VerticalMixFactor + (0.8 - 0.8 * VerticalMixFactor) *
                              GenerateAccretionDiskNoise(vec3(RotPosR, 1.5 * PosTheta, PosY / Rs), 1, 3, 80.0));
                Color0.a   *= (Density); // * (1.0 + VerticalMixFactor);
            }
            if (abs(PosY) < 0.5 * Rs * (1.0 - 5.0 * pow(2.0 * (1.0 - EffectiveRadius), 2.0)))
            {
                DustColor = max(1.0 - pow(PosY / (0.5 * Rs * max(1.0 - 5.0 * pow(2.0 * (1.0 - EffectiveRadius), 2.0), 0.0001)), 2.0), 0.0) * GenerateAccretionDiskNoise(vec3(1.5 * fract((1.5 *  PosThetaForInnerCloud + kPi / HalfPiTimeInside *iTime*TimeRate) / 2.0 / kPi) * 2.0 * kPi, PosR / Rs, PosY / Rs), 0, 6, 80.0);
                Color0 += 0.02 * vec4(vec3(DustColor), 0.2 * DustColor) * sqrt(1.0001 - DirOnDisk.y * DirOnDisk.y) * min(1.0, Dopler * Dopler);
            }
           
            Color =  Color0;
            Color *= 1.0 + 20.0 * exp(-10.0 * (PosR - InterRadius) / (OuterRadius - InterRadius)); // 内侧增加密度

            float BrightWithoutRedshift = 4.5 * DiskTemperature * DiskTemperature * DiskTemperature * DiskTemperature / QuadraticedPeakTemperature;  // 原亮度
            if (DiskTemperature > 1000.0)
            {
                DiskTemperature = max(1000.0, DiskTemperature * RedShift * Dopler * Dopler);
            }

            DiskTemperature = min(100000.0, DiskTemperature);

            Color.xyz *= BrightWithoutRedshift * min(1.0, 1.8 * (OuterRadius - PosR) / (OuterRadius - InterRadius)) *
                         KelvinToRgb(DiskTemperature / exp((PosR - InterRadius) / (0.6 * (OuterRadius - InterRadius))));
            Color.xyz *= min(ShiftMax, RedShift) * min(ShiftMax, Dopler);

            RedShift=min(RedShift,ShiftMax);
            Color.xyz *= pow((1.0 - (1.0 - min(1., RedShift)) * (PosR - InterRadius) / (OuterRadius - InterRadius)), 9.0);
            Color.xyz *= min(1.0, 1.0 + 0.5 * ((PosR - InterRadius) / InterRadius + InterRadius / (PosR - InterRadius)) - max(1.0, RedShift));

            Color *= StepLength / Rs;
        }
    }

    return BaseColor + Color * (1.0 - BaseColor.a);
}
void main()
{
    fragColor      = vec4(0., 0., 0., 0.);
    vec2  FragUv   = gl_FragCoord.xy / iResolution.xy;
    float Fov      = 0.5;
    float TimeRate = 30.;  // 本部分在实际使用时又uniform输入，此外所有iTime*TimeRate应替换为游戏内时间。
    float MBlackHole = 1.49e7;                                                                          // 单位是太阳质量 本部分在实际使用时uniform输入
    float a0         = 0.0;                                                                             // 无量纲自旋系数 本部分在实际使用时uniform输入
    float Rs         = 2. * MBlackHole * kGravityConstant / kSpeedOfLight / kSpeedOfLight * kSolarMass;  // 单位是米 本部分在实际使用时uniform输入
    float z1       = 1. + pow(1. - a0 * a0, 0.333333333333333) * (pow(1. + a0 * a0, 0.333333333333333) + pow(1. - a0, 0.333333333333333));  // 辅助变量      本部分在实际使用时uniform输入
    float RmsRatio = (3. + sqrt(3. * a0 * a0 + z1 * z1) - sqrt((3. - z1) * (3. + z1 + 2. * sqrt(3. * a0 * a0 + z1 * z1)))) / 2.;            // 赤道顺行最内稳定圆轨与Rs之比    本部分在实际使用时uniform输入
    float AccEff   = sqrt(1. - 1. / RmsRatio);                                                                                              // 吸积放能效率,以落到Rms为准 本部分在实际使用时uniform输入

    float mu      = 1.;                                                                            // 吸积物的比荷的倒数,氕为1 本部分在实际使用时uniform输入
    float dmdtEdd = 6.327 * mu / kSpeedOfLight / kSpeedOfLight * MBlackHole * kSolarMass / AccEff;  // 爱丁顿吸积率 本部分在实际使用时uniform输入

    float dmdt = (2e-6) * dmdtEdd;  // 吸积率 本部分在实际使用时uniform输入

    float diskA = 3. * kGravityConstant * kSolarMass / Rs / Rs / Rs * MBlackHole * dmdt / (8. * kPi * kSigma);  // 吸积盘温度系数 本部分在实际使用时uniform输入

    // 计算峰值温度的四次方,用于自适应亮度。峰值温度出现在49InterRadius/36处
    float QuadraticedPeakTemperature = diskA * 0.05665278;  //                                                                                          本部分在实际使用时uniform输入

    Rs         = Rs / kLightYear;       // 单位是ly 本部分在实际使用时uniform输入
    float InterRadius  = 0.7 * RmsRatio * Rs;  // 盘内缘,正常情况下等于最内稳定圆轨
    float OuterRadius = 12. * Rs;             // 盘外缘 本部分在实际使用时uniform输入

    float shiftMax = 1.25;  // 设定一个蓝移的亮度增加上限,以免亮部过于亮

    vec3 WorldUp            = GetCameraRot(vec4(0., 1., 0., 1.)).xyz;
    vec4 BlackHoleAPos     = vec4(0.0, 0.0, 5. * Rs, 1.0);             // 黑洞世界位置 本部分在实际使用时没有
    vec4 BlackHoleADiskNormal = vec4(normalize(vec3(0.2,1.0,0.0)), 1.0);  // 吸积盘世界法向 本部分在实际使用时没有
    // 以下在相机系
    vec3  BlackHoleRPos     = GetCamera(BlackHoleAPos).xyz;         //                                                                                     本部分在实际使用时uniform输入
    vec3  BlackHoleRDiskNormal = GetCameraRot(BlackHoleADiskNormal).xyz;  //                                                                          本部分在实际使用时uniform输入
    vec3  RayDir            = FragUvToDir(FragUv + 0.5 * vec2(RandomStep(FragUv, fract(iTime * 1.0 + 0.5)), RandomStep(FragUv, fract(iTime * 1.0))) / iResolution.xy, Fov);
    vec3  RayPos            = vec3(0.0, 0.0, 0.0);
    
    vec3  PosToBlackHole           = RayPos - BlackHoleRPos;
    float DistanceToBlackHole = length(PosToBlackHole);
    vec3  NormalizedPosToBlackHole      = PosToBlackHole / DistanceToBlackHole;
    
    RayDir=normalize(RayDir-NormalizedPosToBlackHole*dot(NormalizedPosToBlackHole,RayDir)*(-sqrt(max(1.0-Rs*CubicInterpolate(max(min(1.0-(0.01*DistanceToBlackHole/Rs-1.0)/4.0,1.0),0.0))/DistanceToBlackHole,0.00000000000000001))+1.0));
    vec3  LastRayPos;
    vec3  LastRayDir;
    float StepLength = 0.;
    float LastR = length(PosToBlackHole);
    float CosTheta;
    float DeltaPhi;
    float DeltaPhiRate;
    float RayStep;
    bool  flag  = true;
    int   Count = 0;
    while (flag == true)
    {  // 测地raymarching

        PosToBlackHole           = RayPos - BlackHoleRPos;
        DistanceToBlackHole      = length(PosToBlackHole);
        NormalizedPosToBlackHole = PosToBlackHole / DistanceToBlackHole;

        if (DistanceToBlackHole > (2.5 * OuterRadius) && DistanceToBlackHole > LastR && Count > 50)
        {  // 远离黑洞
            flag   = false;
            FragUv = DirToFragUv(RayDir);
        }
        if (DistanceToBlackHole < 0.1 * Rs)
        {
            flag = false;
        }
        if (flag == true)
        {
            fragColor = DiskColor(fragColor, TimeRate, StepLength, RayPos, LastRayPos, RayDir, LastRayDir, WorldUp, BlackHoleRPos, BlackHoleRDiskNormal, Rs, InterRadius, OuterRadius, diskA, QuadraticedPeakTemperature, shiftMax);  // 吸积盘颜色
        }

        if (fragColor.a > 0.99)
        {
            flag = false;
        }
        LastRayPos   = RayPos;
        LastRayDir   = RayDir;
        LastR        = DistanceToBlackHole;
        CosTheta     = length(cross(NormalizedPosToBlackHole, RayDir));                           // 前进方向与切向夹角
        DeltaPhiRate = -1.0 * CosTheta * CosTheta * CosTheta * (1.5 * Rs / DistanceToBlackHole);  // 单位长度光偏折角
        if (Count == 0)
        {
            RayStep = RandomStep(FragUv, fract(iTime * 1.0));  // 光起步步长抖动
        }
        else
        {
            RayStep = 1.0;
        }

        RayStep *= 0.15 + 0.25 * min(max(0.0, 0.5 * (0.5 * DistanceToBlackHole / max(10.0 * Rs, OuterRadius) - 1.0)), 1.0);

        if ((DistanceToBlackHole) >= 2.0 * OuterRadius)
        {
            RayStep *= DistanceToBlackHole;
        }
        else if ((DistanceToBlackHole) >= 1.0 * OuterRadius)
        {
            RayStep *= ((Rs) * (2.0 * OuterRadius - DistanceToBlackHole) +
                        DistanceToBlackHole * (DistanceToBlackHole - OuterRadius)) / OuterRadius;
        }
        else
        {
            RayStep *= min(Rs,DistanceToBlackHole);
        }

        RayPos += RayDir * RayStep;
        DeltaPhi = RayStep / DistanceToBlackHole * DeltaPhiRate;
        RayDir     = normalize(RayDir + (DeltaPhi + DeltaPhi * DeltaPhi * DeltaPhi / 3.0) *
                     cross(cross(RayDir, NormalizedPosToBlackHole), RayDir) / CosTheta);  // 更新方向，里面的（dthe +DeltaPhi^3/3）是tan（dthe）
        StepLength = RayStep;

        Count++;
        // if(DistanceToBlackHole > (100.*Rs) && DistanceToBlackHole > lastR && Count > 50) {
        //     flag = false;
            
        //     // 根据背景类型选择不同的背景
        //     if (backgroundType == 0) { // 棋盘背景
        //         uv = DirToFragUv(RayDir);
        //         fragColor += 0.5 * texture(iChannel1, vec2(fract(uv.x), fract(uv.y)) * (1.0 - fragColor.a));
        //     } else if (backgroundType == 1) { // 纯黑背景
        //         fragColor += vec4(0.0, 0.0, 0.0, 1.0) * (1.0 - fragColor.a);
        //     } else if (backgroundType == 3) { // 使用第一通道的纹理
        //         uv = DirToFragUv(RayDir);
        //         fragColor += 0.5 * texture(backgroundTexture, vec2(fract(uv.x), fract(uv.y)) * (1.0 - fragColor.a));
        //     } else { // 其他背景类型使用棋盘
        //         uv = DirToFragUv(RayDir);
        //         fragColor += 0.5 * texture(iChannel1, vec2(fract(uv.x), fract(uv.y)) * (1.0 - fragColor.a));
        //     }
        // }
        // if(DistanceToBlackHole < 0.1 * Rs) {
        //     flag = false;
        // }
    }
    // 为了套bloom先逆处理一遍
    float colorRFactor = 3.0*fragColor.r / (fragColor.g+fragColor.g+fragColor.b);
    float colorBFactor = 3.0*fragColor.b / (fragColor.g+fragColor.g+fragColor.b);
    float colorGFactor = 3.0*fragColor.g / (fragColor.g+fragColor.g+fragColor.b);

    float bloomMax = 12.0;
    fragColor.r    = min(-4.0 * log(1. - pow(fragColor.r, 2.2)), bloomMax * colorRFactor);
    fragColor.g    = min(-4.0 * log(1. - pow(fragColor.g, 2.2)), bloomMax * colorGFactor);
    fragColor.b    = min(-4.0 * log(1. - pow(fragColor.b, 2.2)), bloomMax * colorBFactor);
    fragColor.a    = min(-4.0 * log(1. - pow(fragColor.a, 2.2)), 4.0);
    // TAA
    float blendWeight = 1.0 - pow(0.5, (iTimeDelta) / max(min((0.131 * 36.0 / (TimeRate) * (GetKeplerianAngularVelocity(3. * 0.00000465, 0.00000465)) / (GetKeplerianAngularVelocity(3. * Rs, Rs))), 0.3),
                                                          0.02));  // 本部分在实际使用时max(min((0.131*36.0/(TimeRate)*(omega(3.*0.00000465,0.00000465))/(omega(3.*Rs,Rs))),0.3),0.02)由uniform输入
    blendWeight       = (iFrame < 2 || iMouse.z > 0.0) ? 1.0 : blendWeight;

    vec4 previousColor = texelFetch(iChannel3, ivec2(gl_FragCoord), 0);                     // 获取前一帧的颜色
    fragColor          = (blendWeight)*fragColor + (1.0 - blendWeight) * previousColor;  // 混合当前帧和前一帧
    //fragColor+=0.5*texelFetch(iChannel1, ivec2(vec2(fract(FragUv.x),fract(FragUv.y))*iChannelResolution.xy), 0)*(1.0-fragColor.a);
    //fragColor.a = 1.0;
}