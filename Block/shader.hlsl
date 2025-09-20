//HLSL �ǡ���������˾���

struct VSInput
{
	float4 position : POSITION;
	float2 texcoordUV : TEXCOORD;
};

struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoordUV : TEXCOORD;
};

cbuffer GlobalData : register(b0, space0)
{
	row_major float4x4 MVP;//�����Ƚ���
}

VSOutput VSMain(VSInput input)
{
	VSOutput output;
	output.position = mul(input.position, MVP);
    //�����ʹ�� row_major float4x4 MVP; �����ֱ��mul(MVP, input.position)
	output.texcoordUV = input.texcoordUV;
	return output;
}


Texture2D m_texture : register(t0, space0);
SamplerState m_sampler : register(s0, space0);

float4 PSMain(VSOutput input) : SV_Target
{
    float4 p = float4(0, 0, 0, 1);
	return m_texture.Sample(m_sampler, input.texcoordUV);
    //return p;
}