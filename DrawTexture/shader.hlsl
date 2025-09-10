
// (4) DrawTexture���� DirectX 12 ��һ����ʯԭ��

struct VSInput      // VS �׶����붥������
{
    float4 position : POSITION;         // ���붥���λ�ã�POSITION �����Ӧ C++ �����벼���е� POSITION
    float2 texcoordUV : TEXCOORD;       // ���붥����������꣬TEXCOORD �����Ӧ C++ �����벼���е� TEXCOORD
};

struct VSOutput     // VS �׶������������
{
    float4 position : SV_Position;      // ��������λ�ã�SV_POSITION ��ϵͳ���壬ָ�����������Ѿ�λ����βü��ռ䣬֪ͨ��դ���׶ζԶ������͸�ӳ�������Ļӳ��
    float2 texcoordUV : TEXCOORD;       // ���������������ʱ����Ȼ��Ҫ TEXCOORD ����
};

// Vertex Shader ������ɫ����ں��� (�𶥵�����)���������� IA �׶�����Ķ������ݣ�����������βü��ռ��µĶ�������
// ��һ�׶Σ�Input Assembler ����װ��׶�
// ��һ�׶Σ�Rasterization ��դ���׶�
VSOutput VSMain(VSInput input)
{
    VSOutput output; // ����ֱ���� IA �׶����붥���� NDC �ռ��µ����꣬��������任��ֱ�Ӹ�ֵ���ؾ���
    output.position = input.position;
    output.texcoordUV = input.texcoordUV;
    
    return output;
}

// register(*#��spaceN) *��ʾ��Դ���ͣ�#��ʾ���õļĴ�����ţ�spaceN ��ʾʹ�õ� N �żĴ����ռ�

Texture2D m_texure : register(t0, space0);          // ����t ��ʾ SRV ��ɫ����Դ��t0 ��ʾ 0 �� SRV �Ĵ�����space0 ��ʾʹ�� t0 �� 0 �ſռ�
SamplerState m_sampler : register(s0, space0);      // �����������s ��ʾ��������s0 ��ʾ 0 �� sampler �Ĵ�����space0 ��ʾʹ�� s0 �� 0 �ſռ�

// Pixel Shader ������ɫ����ں��� (����������)���������Թ�դ���׶ξ�����ֵ���ÿ��ƬԪ������������ɫ
// ��һ�׶Σ�Rasterization ��դ���׶�
// ��һ�׶Σ�Output Merger ����ϲ��׶�
float4 PSMain(VSOutput input) : SV_Target   // SV_Target Ҳ��ϵͳ���壬֪ͨ����ϲ��׶ν� PS �׶η��ص���ɫд�뵽��ȾĿ��(��ɫ����)��
{
    return m_texure.Sample(m_sampler, input.texcoordUV);    // ��������ɫ�����ݹ�դ����ֵ�õ��� UV �����������в���
}

