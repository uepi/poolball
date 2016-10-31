
	cbuffer cbBuffer : register(b0)
{
	// 表示位置とテクセルのオフセット値
	float4 g_Position    : packoffset(c0);
	// 頂点カラー
	float4 g_Color       : packoffset(c1);
};

// テクスチャー
Texture2D g_Tex : register(t0);

// サンプラーステート
SamplerState  g_Sampler : register(s0);

// 頂点シェーダーの入力パラメータ
struct VS_IN
{
	float3 pos   : POSITION;   // 頂点座標
	float2 texel : TEXCOORD0;  // テクセル
};

// 頂点シェーダーの出力パラメータ
struct VS_OUT_PS_IN
{
	float4 pos   : SV_POSITION;
	float2 texel : TEXCOORD0;
	float4 color : TEXCOORD1;  // 頂点カラー
};

// 頂点シェーダー
VS_OUT_PS_IN DebugFont_VS_Main(VS_IN In)
{
	VS_OUT_PS_IN Out;

	float3 v = In.pos + float3(g_Position.xy, 0);
	Out.pos = float4(v, 1);
	Out.texel = In.texel + g_Position.zw;
	Out.color = g_Color;
	return Out;
}

// ピクセルシェーダ
float4 DebugFont_PS_Main(VS_OUT_PS_IN In) : SV_TARGET
{
	float4 col = g_Tex.Sample(g_Sampler, In.texel);
	return float4(col.rgb * In.color.rgb, col.a);
}