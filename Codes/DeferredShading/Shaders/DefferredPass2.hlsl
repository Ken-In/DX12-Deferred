struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction; // directional/spot light only
    float FalloffEnd; // point/spot light only
    float3 Position; // point light only
    float SpotPower; // spot light only
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD0;
    float2 quadID : TEXCOORD1;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};


cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    Light gLights[5];
};

Texture2D gPosMap : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gColorMap : register(t2);

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout = (VertexOut) 0.0f;
    int quadID = vid / 6;
    vid %= 6;
    vout.TexC = gTexCoords[vid];
    vout.PosH = float4(vout.TexC.x * 2 - 1, 1 - vout.TexC.y * 2, 0.0f, 1.0f);
    
    if (quadID >= 1)
    {
        vout.PosH.xy *= 0.25f;
        vout.PosH.x -= (0.75f - 0.5f * quadID);
        vout.PosH.y -= 0.75f;
    }
    
    vout.quadID.x = quadID;
    
    return vout;
}

// 计算菲涅尔项
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightDir)
{
    float cosIncidentAngle = saturate(dot(normal, lightDir));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}


float4 PS(VertexOut pin) : SV_Target
{
    // 从三张图采样
    float4 posAndR0 = gPosMap.Sample(gsamPointClamp, pin.TexC);
    float4 normalAndShininess = gNormalMap.Sample(gsamPointClamp, pin.TexC);
    float3 color = gColorMap.Sample(gsamPointClamp, pin.TexC).xyz;
    
    float3 posW = posAndR0.xyz;
    float3 fresnelR0 = (posAndR0.w);
    float3 normalW = normalAndShininess.xyz;
    float shininess = normalAndShininess.w;
    
    if (pin.quadID.x >= 1)
    {
        if (pin.quadID.x == 1.0f)
            return float4(posW, 1.0f);
        else if (pin.quadID.x == 2.0f)
            return float4(normalW, 1.0f);
        else
            return float4(color, 1.0f);
    }
    
    if(fresnelR0.x == 0.0f)
        return float4(color, 1.0f);
    
    float3 ambient = (float3) gAmbientLight * color;
    
    float3 diffuse = (0.0f);
    float3 specular = (0.0f);
    
    float m = shininess * 256.0f;
    float3 viewDir = normalize(gEyePosW - posW);
    
    for (int i = 0; i < 3; i++)
    {
        // diffuse color
        float3 lightDir = -gLights[i].Direction;
        float3 lightStrength = gLights[i].Strength * max(dot(lightDir, normalW), 0);
        diffuse += color * lightStrength;
        
        // specular
        float3 halfVec = normalize(viewDir + lightDir);
        float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normalW), 0.0f), m) / 8.0f;
        float3 fresnelFactor = SchlickFresnel(fresnelR0, normalW, lightDir);
        specular += lightStrength * fresnelFactor * roughnessFactor;
    }
    // 这部分可能超过区间[0,1], 将其重新映射回0,1区间
    specular = specular / (specular + 1.0f);
    
    float4 lightColor = float4(ambient + diffuse + specular, 1.0f);
    return lightColor;
}


