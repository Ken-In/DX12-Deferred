
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
	
    // �任������ռ�
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // �ǵȱ�����ʱ, ��Ҫ��������������ת�þ���
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
	
	vout.TangentW = mul(vin.TangentU, (float3x3)gWorld);

    // �任�����ÿռ�
    vout.PosH = mul(posW, gViewProj);
	
	// ��������ı任
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

PixelOut PS(VertexOut pin)
{
	// ��ȡ����
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float  roughness = matData.Roughness;
	uint diffuseMapIndex = matData.DiffuseMapIndex;
	uint normalMapIndex = matData.NormalMapIndex;
	
	// �Է��߲�ֵ���ܻ�ʹ��ʧȥ��λ����, ����ٴι�һ��.
    pin.NormalW = normalize(pin.NormalW);
	
	// ���㷨��
	float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);	

	// �õ������յ���ɫ
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    // ����⻬��
	const float shininess = (1.0f - roughness) * normalMapSample.a;
	
    PixelOut pout;
    pout.position = float4(pin.PosW, fresnelR0.x); // �������R0�����Сһ��
    pout.normal = float4(bumpedNormalW.xyz, shininess); // ���ߺ͹⻬��
    pout.color = float4(diffuseAlbedo.xyz, 1.0f);
	
	// ��ɫ��Ҫ���Ϸ���Ĳ���
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
    pout.color += float4(shininess * fresnelFactor * reflectionColor.rgb, 0.0f);
	
    return pout;
}


