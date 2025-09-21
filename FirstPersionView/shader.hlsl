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
    //row_major的作用是使用与DirectXMath相同的数据解释方式
    row_major float4x4 MVP; // MVP 矩阵，用于将顶点坐标从模型空间变换到齐次裁剪空间，HLSL 默认按列存储，row_major 表示数据按行存储
};
/*
内部算法的实现

     
            1,0,0 行向量  (事实上为列向量)    
1,1,1   *   0,1,0 行向量  = (1,1,1 * 1,0,0),(1,1,1 * 0,1,0),(1,1,1 * 0,0,1)
            0,0,1 行向量



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