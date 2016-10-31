#include "../poolbool/UDebugFont..h"

/*
cbuffer cbBuffer : register( b0 )
{
// �\���ʒu�ƃe�N�Z���̃I�t�Z�b�g�l
float4 g_Position    : packoffset( c0 );
// ���_�J���[
float4 g_Color       : packoffset( c1 );
};

// �e�N�X�`���[
Texture2D g_Tex : register( t0 );

// �T���v���[�X�e�[�g
SamplerState  g_Sampler : register( s0 );

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
VS_OUT_PS_IN DebugFont_VS_Main( VS_IN In )
{
VS_OUT_PS_IN Out;

float3 v = In.pos + float3( g_Position.xy, 0 );
Out.pos   = float4( v, 1 );
Out.texel = In.texel + g_Position.zw;
Out.color = g_Color;
return Out;
}

// �s�N�Z���V�F�[�_
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

	// �V�F�[�_�[���쐬����
	m_pGraphicsPipeline->CreateVertexShaderFromFile(pD3DDevice, L"BesicVS.hlsl","DebugFont_VS_Main",layout,ARRAYSIZE(layout));
	m_pGraphicsPipeline->CreatePixelShaderFromFile(pD3DDevice, L"BesicVS.hlsl", "DebugFont_PS_Main");

	// ���X�^���C�U�[�X�e�[�g���쐬����
	m_pGraphicsPipeline->CreateRasterizerState(pD3DDevice, D3D11_CULL_MODE::D3D11_CULL_BACK);

	// �[�x�X�e���V���X�e�[�g���쐬����
	m_pGraphicsPipeline->CreateDepthStencilState(pD3DDevice, FALSE, D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL);

	// �u�����h�X�e�[�g����`�����ō쐬����
	UGraphicsPipeline::UEBLEND_STATE BlendStateType[1] = { UGraphicsPipeline::UEBLEND_STATE::ALIGNMENT };
	m_pGraphicsPipeline->CreateBlendState(pD3DDevice, BlendStateType, 1);

	m_FontWidth = FontWidth;             // �|���S���̉���
	m_FontHeight = FontHeight;            // �|���S���̏c��
	float TU = 1.0f / (float)m_FontCnt;   // 1�t�H���g������̃e�N�Z���̉���

										  // ���_�̃f�[�^
	UDebugFont::VERTEX v[] = {
		XMFLOAT3(m_FontWidth * 0.5f,  m_FontHeight * 0.5f, 0), XMFLOAT2(TU, 0),
		XMFLOAT3(-m_FontWidth * 0.5f,  m_FontHeight * 0.5f, 0), XMFLOAT2(0, 0),
		XMFLOAT3(m_FontWidth * 0.5f, -m_FontHeight * 0.5f, 0), XMFLOAT2(TU, 1),
		XMFLOAT3(-m_FontWidth * 0.5f, -m_FontHeight * 0.5f, 0), XMFLOAT2(0, 1)
	};
	// ���_�o�b�t�@���쐬����
	m_pVertexBuffer = UBuffers::CreateVertexBuffer(pD3DDevice, v, sizeof(v), 0);

	// �f�J�[���}�b�v�����[�h
	UMaps::CreateSRViewFromDDSFile(pD3DDevice, _T("../Resource\\DebugFont.dds"), &m_pSRView);

	// �T���v���[�X�e�[�g���쐬����
	m_pSamplerState = USamplers::CreateSamplerState(pD3DDevice, D3D11_TEXTURE_ADDRESS_CLAMP);

	// �萔�o�b�t�@���쐬����
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

	// �e��O���t�B�b�N�p�C�v���C����ݒ�
	m_pGraphicsPipeline->SetVertexShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetHullShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetDomainShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetGeometryShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetPixelShader(pD3DDeviceContext);
	m_pGraphicsPipeline->SetRasterizerState(pD3DDeviceContext);
	m_pGraphicsPipeline->SetDepthStencilState(pD3DDeviceContext);
	m_pGraphicsPipeline->SetBlendState(pD3DDeviceContext);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(UDebugFont::VERTEX);
	UINT offset = 0;
	pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// �v���~�e�B�u �^�C�v����уf�[�^�̏����Ɋւ������ݒ�
	pD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �s�N�Z���V�F�[�_�[�̃T���v���[�X�e�[�g��ݒ肷��
	pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);

	// �f�J�[���}�b�v��ݒ肷��
	pD3DDeviceContext->PSSetShaderResources(0, 1, &m_pSRView);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	UDebugFont::CONSTANT_BUFFER* cBuffer;

	// �����������[�v
	for (int i = 0; i<(int)_tcsclen(m_pStr); i++)
	{
		if (m_pStr[i] == '\n')
		{
			pos.x = m_Position.x;
			pos.y -= m_FontHeight;
			continue;
		}

		if (FAILED(hr = pD3DDeviceContext->Map(m_pConstantBuffers, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			throw(UException(hr, _T("UDebugFont::RenderDebugText()�ŃG���[���������܂����B���b�N�ł��܂���")));

		cBuffer = (UDebugFont::CONSTANT_BUFFER*)(mappedResource.pData);

		// �\���ʒu
		::CopyMemory(&cBuffer->Position, &pos, sizeof(XMFLOAT2));

		// �e�N�Z��
		cBuffer->TexelOffset.x = (float)(m_pStr[i] - 32) / m_FontCnt; // TU
		cBuffer->TexelOffset.y = 0.0f;                                  // TV

																		// ���_�J���[
		::CopyMemory(&cBuffer->Color, &m_Color, sizeof(XMFLOAT4));

		pD3DDeviceContext->Unmap(m_pConstantBuffers, 0);

		// ���_�V�F�[�_�[�ɒ萔�o�b�t�@��ݒ肷��B
		pD3DDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffers);

		// �s�N�Z���V�F�[�_�[�ɒ萔�o�b�t�@��ݒ肷��B
		pD3DDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffers);

		// �`��
		pD3DDeviceContext->Draw(4, 0);

		pos.x += m_FontWidth;
	}
}


// **********************************************************************************
// FPS�`��N���X
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
