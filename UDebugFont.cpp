#include "../poolbool/UDebugFont..h"

/*
cbuffer cbBuffer : register( b0 )
{
// 表示位置とテクセルのオフセット値
float4 g_Position    : packoffset( c0 );
// 頂点カラー
float4 g_Color       : packoffset( c1 );
};

// テクスチャー
Texture2D g_Tex : register( t0 );

// サンプラーステート
SamplerState  g_Sampler : register( s0 );

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
VS_OUT_PS_IN DebugFont_VS_Main( VS_IN In )
{
VS_OUT_PS_IN Out;

float3 v = In.pos + float3( g_Position.xy, 0 );
Out.pos   = float4( v, 1 );
Out.texel = In.texel + g_Position.zw;
Out.color = g_Color;
return Out;
}

// ピクセルシェーダ
float4 DebugFont_PS_Main( VS_OUT_PS_IN In ) : SV_TARGET
{
float4 col = g_Tex.Sample( g_Sampler, In.texel );
return float4( col.rgb * In.color.rgb, col.a );
}
*/

UDebugFont::UDebugFont()
{
	m_pVertexBuffer = nullptr;
	m_pSRView = nullptr;
	m_pSamplerState = nullptr;
	m_pConstantBuffers = nullptr;
	m_pGraphicsPipeline = nullptr;
	m_pStr[0] = '\0';
}

UDebugFont::~UDebugFont()
{
	Invalidate();
}

void UDebugFont::Invalidate()
{
	m_pStr[0] = '\0';
	SAFE_DELETE(m_pGraphicsPipeline);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pSRView);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pConstantBuffers);
}

void UDebugFont::CreateMesh(ID3D11Device* pD3DDevice, float FontWidth, float FontHeight)
{
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_pGraphicsPipeline = NEW UGraphicsPipeline();

	// シェーダーを作成する
	m_pGraphicsPipeline->CreateVertexShaderFromFile(pD3DDevice, L"BesicVS.hlsl","DebugFont_VS_Main",layout,ARRAYSIZE(layout));
	m_pGraphicsPipeline->CreatePixelShaderFromFile(pD3DDevice, L"BesicVS.hlsl", "DebugFont_PS_Main");

	// ラスタライザーステートを作成する
	m_pGraphicsPipeline->CreateRasterizerState(pD3DDevice, D3D11_CULL_MODE::D3D11_CULL_BACK);

	// 深度ステンシルステートを作成する
	m_pGraphicsPipeline->CreateDepthStencilState(pD3DDevice, FALSE, D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL);

	// ブレンドステートを線形合成で作成する
	UGraphicsPipeline::UEBLEND_STATE BlendStateType[1] = { UGraphicsPipeline::UEBLEND_STATE::ALIGNMENT };
	m_pGraphicsPipeline->CreateBlendState(pD3DDevice, BlendStateType, 1);

	m_FontWidth = FontWidth;             // ポリゴンの横幅
	m_FontHeight = FontHeight;            // ポリゴンの縦幅
	float TU = 1.0f / (float)m_FontCnt;   // 1フォント当たりのテクセルの横幅

										  // 頂点のデータ
	UDebugFont::VERTEX v[] = {
		XMFLOAT3(m_FontWidth * 0.5f,  m_FontHeight * 0.5f, 0), XMFLOAT2(TU, 0),
		XMFLOAT3(-m_FontWidth * 0.5f,  m_FontHeight * 0.5f, 0), XMFLOAT2(0, 0),
		XMFLOAT3(m_FontWidth * 0.5f, -m_FontHeight * 0.5f, 0), XMFLOAT2(TU, 1),
		XMFLOAT3(-m_FontWidth * 0.5f, -m_FontHeight * 0.5f, 0), XMFLOAT2(0, 1)
	};
	// 頂点バッファを作成する
	m_pVertexBuffer = UBuffers::CreateVertexBuffer(pD3DDevice, v, sizeof(v), 0);

	// デカールマップをロード
	UMaps::CreateSRViewFromDDSFile(pD3DDevice, _T("../Resource\\DebugFont.dds"), &m_pSRView);

	// サンプラーステートを作成する
	m_pSamplerState = USamplers::CreateSamplerState(pD3DDevice, D3D11_TEXTURE_ADDRESS_CLAMP);

	// 定数バッファを作成する
	m_pConstantBuffers = m_pGraphicsPipeline->CreateConstantBuffer(pD3DDevice, nullptr, sizeof(UDebugFont::CONSTANT_BUFFER), D3D11_CPU_ACCESS_WRITE);
}

void UDebugFont::NextFrame(TCHAR* pStr, XMFLOAT2* pPosition, XMFLOAT4* pColor)
{
	_tcscpy_s(m_pStr, pStr);
	::CopyMemory(&m_Position, pPosition, sizeof(XMFLOAT2));
	::CopyMemory(&m_Color, pColor, sizeof(XMFLOAT4));
}

void UDebugFont::Render(ID3D11DeviceContext* pD3DDeviceContext)
{
	HRESULT hr = E_FAIL;

	if (_tcsclen(m_pStr) == 0)
		return;

	XMFLOAT2 pos = m_Position;

	// 各種グラフィックパイプラインを設定
	m_pGraphicsPipeline->SetVertexShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetHullShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetDomainShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetGeometryShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetPixelShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetRasterizerState(pD3DDeviceContext);
	m_pGraphicsPipeline->SetDepthStencilState(pD3DDeviceContext);
	m_pGraphicsPipeline->SetBlendState(pD3DDeviceContext);

	// 頂点バッファ設定
	UINT stride = sizeof(UDebugFont::VERTEX);
	UINT offset = 0;
	pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// プリミティブ タイプおよびデータの順序に関する情報を設定
	pD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ピクセルシェーダーのサンプラーステートを設定する
	pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);

	// デカールマップを設定する
	pD3DDeviceContext->PSSetShaderResources(0, 1, &m_pSRView);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	UDebugFont::CONSTANT_BUFFER* cBuffer;

	// 文字数分ループ
	for (int i = 0; i<(int)_tcsclen(m_pStr); i++)
	{
		if (m_pStr[i] == '\n')
		{
			pos.x = m_Position.x;
			pos.y -= m_FontHeight;
			continue;
		}

		if (FAILED(hr = pD3DDeviceContext->Map(m_pConstantBuffers, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			throw(UException(hr, _T("UDebugFont::RenderDebugText()でエラーが発生しました。ロックできません")));

		cBuffer = (UDebugFont::CONSTANT_BUFFER*)(mappedResource.pData);

		// 表示位置
		::CopyMemory(&cBuffer->Position, &pos, sizeof(XMFLOAT2));

		// テクセル
		cBuffer->TexelOffset.x = (float)(m_pStr[i] - 32) / m_FontCnt; // TU
		cBuffer->TexelOffset.y = 0.0f;                                  // TV

																		// 頂点カラー
		::CopyMemory(&cBuffer->Color, &m_Color, sizeof(XMFLOAT4));

		pD3DDeviceContext->Unmap(m_pConstantBuffers, 0);

		// 頂点シェーダーに定数バッファを設定する。
		pD3DDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffers);

		// ピクセルシェーダーに定数バッファを設定する。
		pD3DDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffers);

		// 描画
		pD3DDeviceContext->Draw(4, 0);

		pos.x += m_FontWidth;
	}
}


// **********************************************************************************
// FPS描画クラス
// **********************************************************************************

UFPS::UFPS()
{
	m_PrevTime = ::timeGetTime();
	m_FrameCounter = 0;
	m_Frame = 0;
}

UFPS::~UFPS()
{
	Invalidate();
}

void UFPS::Invalidate()
{
	UDebugFont::Invalidate();
}

void UFPS::CreateMesh(ID3D11Device* pD3DDevice)
{
	UDebugFont::CreateMesh(pD3DDevice, 0.03f, 0.08f);
}

void UFPS::NextFrame()
{
	DWORD now = ::timeGetTime();
	TCHAR fps[20];

	if (now - m_PrevTime > 1000)
	{
		m_PrevTime = now;
		m_Frame = m_FrameCounter;
		m_FrameCounter = 0;
	}
	else
		m_FrameCounter++;

	_stprintf_s(fps, _T("FPS : %d"), m_Frame);

	UDebugFont::NextFrame(fps, &XMFLOAT2(-0.97f, 0.95f), &XMFLOAT4(1, 1, 1, 1));
}

void UFPS::Render(ID3D11DeviceContext* pD3DDeviceContext)
{
	UDebugFont::Render(pD3DDeviceContext);
}
