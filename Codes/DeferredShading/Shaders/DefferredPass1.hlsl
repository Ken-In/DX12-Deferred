
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
};

struct PixelOut
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 color : SV_Target2;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;
    
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;
    vout.PosH = mul(posW, gViewProj);

    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);
	
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);
    
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

PixelOut PS(VertexOut pin)
{
	// 获取材质
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    // 法线插值可能会使其失去单位长度, 因此再次归一化.
    pin.NormalW = normalize(pin.NormalW);
    
    // 计算法线
    float4 normalSample = gTextureMaps[matData.NormalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalSample.xyz, pin.NormalW, pin.TangentW);
   
    // 材质颜色乘纹理颜色 得到的基本颜色
    float4 diffuseAlbedo = matData.DiffuseAlbedo * 
    gTextureMaps[matData.DiffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
    // 计算光滑度 normalMap的a通道存的是光滑度
    const float shininess = (1.0f - matData.Roughness) * normalSample.a;
    
    PixelOut pout;
    pout.position = float4(pin.PosW, matData.FresnelR0.x); // 假设R0三项大小一致
    pout.normal = float4(bumpedNormalW, shininess); // normalbuffer a通道存光滑度
    
    // 基本颜色 + 反射部分颜色
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    float3 ref = reflect(-toEyeW, bumpedNormalW);
    float4 reflectColor = gCubeMap.Sample(gsamAnisotropicWrap, ref);
    float3 fresnelFactor = SchlickFresnel(matData.FresnelR0, bumpedNormalW, ref);
    pout.color = float4(diffuseAlbedo.xyz +shininess * fresnelFactor * reflectColor.xyz, 1.0f);
    
    return pout;
}


