#include "UGraphicsPipeline.h"

UGraphicsPipeline::UGraphicsPipeline()
{
	m_pLayout = nullptr;
	m_pVertexShader = nullptr;
	m_pHullShader = nullptr;
	m_pDomainShader = nullptr;
	m_pGeometryShader = nullptr;
	m_pPixelShader = nullptr;
	m_pRasterizerState = nullptr;
	m_pDepthStencilState = nullptr;
	m_pBlendState = nullptr;
}

UGraphicsPipeline::~UGraphicsPipeline()
{
	Invalidate();
}

void UGraphicsPipeline::Invalidate()
{
	SAFE_RELEASE(m_pLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pHullShader);
	SAFE_RELEASE(m_pDomainShader);
	SAFE_RELEASE(m_pGeometryShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pRasterizerState);
	SAFE_RELEASE(m_pDepthStencilState);
	SAFE_RELEASE(m_pBlendState);
}

// ���_�V�F�[�_�[���t�@�C������쐬����
void UGraphicsPipeline::CreateVertexShaderFromFile(ID3D11Device* pD3DDevice,
	TCHAR pSrcFile[], CHAR pFunctionName[],
	D3D11_INPUT_ELEMENT_DESC pLayoutDesc[], UINT NumElements)
{
	HRESULT hr = E_FAIL;
	CHAR* Profile = "vs_5_0";

	ID3DBlob* pBlob = nullptr;

	// �s����D��Őݒ肵�A�Â��`���̋L�q�������Ȃ��悤�ɂ���
	UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
	// �œK�����x����ݒ肷��
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	__try
	{
		// �t�@�C�������ɃG�t�F�N�g���R���p�C������
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreateVertexShaderFromFile()�ŃG���[���������܂����B�G�t�F�N�g�t�@�C���̃R���p�C���G���[�ł��B -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// ���_�V�F�[�_�[������������쐬����
		CreateVertexShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pLayoutDesc, NumElements);
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// ���_�V�F�[�_�[������������쐬����
void UGraphicsPipeline::CreateVertexShaderFromMemory(ID3D11Device* pD3DDevice,
	BYTE* pShader, size_t size,
	D3D11_INPUT_ELEMENT_DESC m_pLayoutDesc[], UINT NumElements)
{
	HRESULT hr = E_FAIL;

	// �R���p�C���ς݃V�F�[�_�[����A���_�V�F�[�_�[ �I�u�W�F�N�g���쐬����
	if (FAILED(hr = pD3DDevice->CreateVertexShader(pShader, size, nullptr, &m_pVertexShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateVertexShaderFromMemory()�ŃG���[���������܂����B���_�V�F�[�_�[�̍쐬�G���[�ł��B")));

	// �V�F�[�_�[�Ŏg�p�������̓o�b�t�@�[���L�q���邽�߂̓��̓��C�A�E�g�I�u�W�F�N�g���쐬����B
	if (FAILED(hr = pD3DDevice->CreateInputLayout(m_pLayoutDesc, NumElements, pShader, size, &m_pLayout)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateVertexShaderFromMemory()�ŃG���[���������܂����B���̓��C�A�E�g�̍쐬�G���[�ł��B")));
}



// �W�I���g���V�F�[�_�[���t�@�C������쐬����
void UGraphicsPipeline::CreateGeometryShaderFromFile(ID3D11Device* pD3DDevice, TCHAR pSrcFile[], CHAR  pFunctionName[])
{
	HRESULT hr = E_FAIL;
	CHAR* Profile = "gs_5_0";

	ID3DBlob* pBlob = nullptr;

	UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	__try
	{
		// �t�@�C�������ɃG�t�F�N�g���R���p�C������
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreateGeometryShaderFromFile()�ŃG���[���������܂����B�G�t�F�N�g�t�@�C���̃R���p�C���G���[�ł��B -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// �s�N�Z���V�F�[�_�[������������쐬����
		CreateGeometryShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// �W�I���g���V�F�[�_�[������������쐬����
void UGraphicsPipeline::CreateGeometryShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size)
{
	HRESULT hr = E_FAIL;

	// �R���p�C���ς݃V�F�[�_�[����A�W�I���g���V�F�[�_�[ �I�u�W�F�N�g���쐬����
	if (FAILED(hr = pD3DDevice->CreateGeometryShader(pShader, size, nullptr, &m_pGeometryShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateGeometryShaderFromMemory()�ŃG���[���������܂����B�W�I���g���V�F�[�_�[�̍쐬�G���[�ł��B")));
}

// �s�N�Z���V�F�[�_�[���t�@�C������쐬����
void UGraphicsPipeline::CreatePixelShaderFromFile(ID3D11Device* pD3DDevice,
	TCHAR pSrcFile[],
	CHAR  pFunctionName[]
)
{
	HRESULT hr = E_FAIL;
	CHAR* Profile = "ps_5_0";

	ID3DBlob* pBlob = nullptr;

	UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	__try
	{
		// �t�@�C�������ɃG�t�F�N�g���R���p�C������
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreatePixelShaderFromFile()�ŃG���[���������܂����B�G�t�F�N�g�t�@�C���̃R���p�C���G���[�ł��B -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// �s�N�Z���V�F�[�_�[������������쐬����
		CreatePixelShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// �s�N�Z���V�F�[�_�[������������쐬����
void UGraphicsPipeline::CreatePixelShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size)
{
	HRESULT hr = E_FAIL;

	// �R���p�C���ς݃V�F�[�_�[����A�s�N�Z���V�F�[�_�[ �I�u�W�F�N�g���쐬����
	if (FAILED(hr = pD3DDevice->CreatePixelShader(pShader, size, nullptr, &m_pPixelShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreatePixelShaderFromMemory()�ŃG���[���������܂����B�s�N�Z���V�F�[�_�[�̍쐬�G���[�ł��B")));
}

// ���X�^���C�U�X�e�[�g���쐬
void UGraphicsPipeline::CreateRasterizerState(ID3D11Device* pD3D11Device, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC RasterizerDesc;

	::ZeroMemory(&RasterizerDesc, sizeof(RasterizerDesc));
	RasterizerDesc.FillMode = FillMode;            // �|���S���ʕ`��
	RasterizerDesc.CullMode = CullMode;            // �w��̕����������Ă���O�p�`���J�����O����
	RasterizerDesc.FrontCounterClockwise = TRUE;   // �����v����\��
	RasterizerDesc.DepthBias = 0;
	RasterizerDesc.DepthBiasClamp = 0;
	RasterizerDesc.SlopeScaledDepthBias = 0;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.ScissorEnable = FALSE;          // �V�U�[��`����
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;

	SAFE_RELEASE(m_pRasterizerState);

	if (FAILED(hr = pD3D11Device->CreateRasterizerState(&RasterizerDesc, &m_pRasterizerState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateRasterizerState()�ŃG���[���������܂����BID3D11RasterizerState�C���^�[�t�F�[�X�̍쐬�G���[�ł��B")));
}

// �[�x�X�e���V���X�e�[�g���쐬
void UGraphicsPipeline::CreateDepthStencilState(ID3D11Device* pD3DDevice, BOOL DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteEnabled, D3D11_COMPARISON_FUNC DepthFunc)
{
	HRESULT hr;

	D3D11_DEPTH_STENCIL_DESC DSDesc;

	DSDesc.DepthEnable = DepthEnable;           // �[�x�e�X�g���s����
	DSDesc.DepthWriteMask = DepthWriteEnabled;  // �[�x�o�b�t�@�ւ̏������݂��s����
	DSDesc.DepthFunc = DepthFunc;
	DSDesc.StencilEnable = FALSE;

	SAFE_RELEASE(m_pDepthStencilState);

	if (FAILED(hr = pD3DDevice->CreateDepthStencilState(&DSDesc, &m_pDepthStencilState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateDepthStencilState()�ŃG���[���������܂����BID3D11DepthStencilState�C���^�[�t�F�[�X�̍쐬�G���[�ł��B")));
}

// �u�����h�X�e�[�g���쐬
void UGraphicsPipeline::CreateBlendState(ID3D11Device* pD3DDevice, UEBLEND_STATE* BlendStateType, UINT BlendStateTypeLength, BOOL AlphaToCoverageEnable)
{
	HRESULT hr;
	D3D11_BLEND_DESC BlendDesc;

	if (BlendStateTypeLength == 0 || BlendStateTypeLength >= 8)
		throw(UException(-1, _T("UGraphicsPipeline::CreateBlendState()�ŃG���[���������܂����B���������s���ł��B")));

	::ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = AlphaToCoverageEnable;

	if (BlendStateTypeLength == 1)
		BlendDesc.IndependentBlendEnable = FALSE;
	else
		BlendDesc.IndependentBlendEnable = TRUE;

	for (UINT i = 0; i<BlendStateTypeLength; i++)
	{
		switch (BlendStateType[i])
		{
		case UEBLEND_STATE::NONE:
			BlendDesc.RenderTarget[i].BlendEnable = FALSE;
			BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;

			// ���Z����
		case UEBLEND_STATE::ADD:
			BlendDesc.RenderTarget[i].BlendEnable = TRUE;
			BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;

			// ���`����
		case UEBLEND_STATE::ALIGNMENT:
			BlendDesc.RenderTarget[i].BlendEnable = TRUE;
			BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;

			// �[�x�e�X�g
		case UEBLEND_STATE::DEPTH_TEST:
			BlendDesc.RenderTarget[i].BlendEnable = TRUE;
			BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_MIN;      // �őO�ʂ̐[�x�l�ŏ㏑�����邽�߂ɍŏ��l��I��
			BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_MIN;
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		}
	}

	SAFE_RELEASE(m_pBlendState);

	if (FAILED(hr = pD3DDevice->CreateBlendState(&BlendDesc, &m_pBlendState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateBlendState()�ŃG���[���������܂����BID3D11BlendState�C���^�[�t�F�[�X�̍쐬�G���[�ł��B")));
}

// �萔�o�b�t�@���쐬����
ID3D11Buffer* UGraphicsPipeline::CreateConstantBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag)
{
	HRESULT hr = E_FAIL;

	ID3D11Buffer* pBuffer = nullptr;

	// �o�b�t�@�[ ���\�[�X�B
	D3D11_BUFFER_DESC BufferDesc = { 0 };

	// �T�u���\�[�X
	D3D11_SUBRESOURCE_DATA* resource = nullptr;

	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	UINT CPUAccessFlags = 0;

	__try
	{
		switch (CPUAccessFlag)
		{
			// CPU�A�N�Z�X�������Ȃ�
		case 0:
			Usage = D3D11_USAGE_DEFAULT;
			CPUAccessFlags = CPUAccessFlag;
			break;
			// CPU�A�N�Z�X��������
		default:
			Usage = D3D11_USAGE_DYNAMIC;
			CPUAccessFlags = CPUAccessFlag;
			break;
		}

		// �����l��ݒ肷��
		if (pData)
		{
			resource = new D3D11_SUBRESOURCE_DATA();
			resource->pSysMem = pData;
			resource->SysMemPitch = 0;
			resource->SysMemSlicePitch = 0;
		}

		// �o�b�t�@�̐ݒ�
		BufferDesc.ByteWidth = size;                       // �o�b�t�@�T�C�Y
		BufferDesc.Usage = Usage;                      // ���\�[�X�g�p�@����肷��
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // �o�b�t�@�̎��
		BufferDesc.CPUAccessFlags = CPUAccessFlags;             // CPU �A�N�Z�X
		BufferDesc.MiscFlags = 0;                          // ���̑��̃t���O���ݒ肵�Ȃ�

														   // �o�b�t�@���쐬����
		hr = pD3DDevice->CreateBuffer(&BufferDesc, resource, &pBuffer);
		if (FAILED(hr))
			throw(UException(hr, _T("UGraphicsPipeline::CreateConstantBuffer()�ŃG���[���������܂����B�o�b�t�@�쐬�G���[�ł��B")));
	}
	__finally
	{
		SAFE_DELETE(resource);
	}

	return pBuffer;
}
