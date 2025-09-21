struct VSInput
{
    float4 position : POSITION;
    float4 texcoordUV : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 texcoordUV : TEXCOORD;
};

cbuffer GloableData : register(b0,space0)
{
    //row_major��������ʹ����DirectXMath��ͬ�����ݽ��ͷ�ʽ
    row_major float4x4 MVP; // MVP �������ڽ����������ģ�Ϳռ�任����βü��ռ䣬HLSL Ĭ�ϰ��д洢��row_major ��ʾ���ݰ��д洢
};
/*
�ڲ��㷨��ʵ��

     
            1,0,0 ������  (��ʵ��Ϊ������)    
1,1,1   *   0,1,0 ������  = (1,1,1 * 1,0,0),(1,1,1 * 0,1,0),(1,1,1 * 0,0,1)
            0,0,1 ������



*/
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, MVP);
    output.texcoordUV = input.texcoordUV;
    return output;
}


Texture2D m_texture : register(t0,space0);
SamplerState m_sampler : register(s0,space0);

float4 PSMain(VSOutput input) : SV_TARGET
{
    return m_texture.Sample(m_sampler, input.texcoordUV.xy);
}