
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
	// ��ȡ����
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    // ���߲�ֵ���ܻ�ʹ��ʧȥ��λ����, ����ٴι�һ��.
    pin.NormalW = normalize(pin.NormalW);
    
    // ���㷨��
    float4 normalSample = gTextureMaps[matData.NormalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalSample.xyz, pin.NormalW, pin.TangentW);
   
    // ������ɫ��������ɫ �õ��Ļ�����ɫ
    float4 diffuseAlbedo = matData.DiffuseAlbedo * 
    gTextureMaps[matData.DiffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
    // ����⻬�� normalMap��aͨ������ǹ⻬��
    const float shininess = (1.0f - matData.Roughness) * normalSample.a;
    
    PixelOut pout;
    pout.position = float4(pin.PosW, matData.FresnelR0.x); // ����R0�����Сһ��
    pout.normal = float4(bumpedNormalW, shininess); // normalbuffer aͨ����⻬��
    
    // ������ɫ + ���䲿����ɫ
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    float3 ref = reflect(-toEyeW, bumpedNormalW);
    float4 reflectColor = gCubeMap.Sample(gsamAnisotropicWrap, ref);
    float3 fresnelFactor = SchlickFresnel(matData.FresnelR0, bumpedNormalW, ref);
    pout.color = float4(diffuseAlbedo.xyz +shininess * fresnelFactor * reflectColor.xyz, 1.0f);
    
    return pout;
}


