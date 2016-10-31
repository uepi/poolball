
	cbuffer cbBuffer : register(b0)
{
	// �\���ʒu�ƃe�N�Z���̃I�t�Z�b�g�l
	float4 g_Position    : packoffset(c0);
	// ���_�J���[
	float4 g_Color       : packoffset(c1);
};

// �e�N�X�`���[
Texture2D g_Tex : register(t0);

// �T���v���[�X�e�[�g
SamplerState  g_Sampler : register(s0);

// ���_�V�F�[�_�[�̓��̓p�����[�^
struct VS_IN
{
	float3 pos   : POSITION;   // ���_���W
	float2 texel : TEXCOORD0;  // �e�N�Z��
};

// ���_�V�F�[�_�[�̏o�̓p�����[�^
struct VS_OUT_PS_IN
{
	float4 pos   : SV_POSITION;
	float2 texel : TEXCOORD0;
	float4 color : TEXCOORD1;  // ���_�J���[
};

// ���_�V�F�[�_�[
VS_OUT_PS_IN DebugFont_VS_Main(VS_IN In)
{
	VS_OUT_PS_IN Out;

	float3 v = In.pos + float3(g_Position.xy, 0);
	Out.pos = float4(v, 1);
	Out.texel = In.texel + g_Position.zw;
	Out.color = g_Color;
	return Out;
}

// �s�N�Z���V�F�[�_
float4 DebugFont_PS_Main(VS_OUT_PS_IN In) : SV_TARGET
{
	float4 col = g_Tex.Sample(g_Sampler, In.texel);
	return float4(col.rgb * In.color.rgb, col.a);
}