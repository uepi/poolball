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

// 頂点シェーダーをファイルから作成する
void UGraphicsPipeline::CreateVertexShaderFromFile(ID3D11Device* pD3DDevice,
	TCHAR pSrcFile[], CHAR pFunctionName[],
	D3D11_INPUT_ELEMENT_DESC pLayoutDesc[], UINT NumElements)
{
	HRESULT hr = E_FAIL;
	CHAR* Profile = "vs_5_0";

	ID3DBlob* pBlob = nullptr;

	// 行列を列優先で設定し、古い形式の記述を許可しないようにする
	UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
	// 最適化レベルを設定する
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	__try
	{
		// ファイルを元にエフェクトをコンパイルする
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreateVertexShaderFromFile()でエラーが発生しました。エフェクトファイルのコンパイルエラーです。 -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// 頂点シェーダーをメモリから作成する
		CreateVertexShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pLayoutDesc, NumElements);
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// 頂点シェーダーをメモリから作成する
void UGraphicsPipeline::CreateVertexShaderFromMemory(ID3D11Device* pD3DDevice,
	BYTE* pShader, size_t size,
	D3D11_INPUT_ELEMENT_DESC m_pLayoutDesc[], UINT NumElements)
{
	HRESULT hr = E_FAIL;

	// コンパイル済みシェーダーから、頂点シェーダー オブジェクトを作成する
	if (FAILED(hr = pD3DDevice->CreateVertexShader(pShader, size, nullptr, &m_pVertexShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateVertexShaderFromMemory()でエラーが発生しました。頂点シェーダーの作成エラーです。")));

	// シェーダーで使用される入力バッファーを記述するための入力レイアウトオブジェクトを作成する。
	if (FAILED(hr = pD3DDevice->CreateInputLayout(m_pLayoutDesc, NumElements, pShader, size, &m_pLayout)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateVertexShaderFromMemory()でエラーが発生しました。入力レイアウトの作成エラーです。")));
}



// ジオメトリシェーダーをファイルから作成する
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
		// ファイルを元にエフェクトをコンパイルする
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreateGeometryShaderFromFile()でエラーが発生しました。エフェクトファイルのコンパイルエラーです。 -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// ピクセルシェーダーをメモリから作成する
		CreateGeometryShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// ジオメトリシェーダーをメモリから作成する
void UGraphicsPipeline::CreateGeometryShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size)
{
	HRESULT hr = E_FAIL;

	// コンパイル済みシェーダーから、ジオメトリシェーダー オブジェクトを作成する
	if (FAILED(hr = pD3DDevice->CreateGeometryShader(pShader, size, nullptr, &m_pGeometryShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateGeometryShaderFromMemory()でエラーが発生しました。ジオメトリシェーダーの作成エラーです。")));
}

// ピクセルシェーダーをファイルから作成する
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
		// ファイルを元にエフェクトをコンパイルする
		if (FAILED(hr = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, Profile, Flag1, 0, &pBlob, nullptr)))
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("UGraphicsPipeline::CreatePixelShaderFromFile()でエラーが発生しました。エフェクトファイルのコンパイルエラーです。 -> %s"), pSrcFile);
			throw(UException(hr, errstr));
		}

		// ピクセルシェーダーをメモリから作成する
		CreatePixelShaderFromMemory(pD3DDevice, (LPBYTE)pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	}
	__finally
	{
		SAFE_RELEASE(pBlob);
	}
}

// ピクセルシェーダーをメモリから作成する
void UGraphicsPipeline::CreatePixelShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size)
{
	HRESULT hr = E_FAIL;

	// コンパイル済みシェーダーから、ピクセルシェーダー オブジェクトを作成する
	if (FAILED(hr = pD3DDevice->CreatePixelShader(pShader, size, nullptr, &m_pPixelShader)))
		throw(UException(hr, _T("UGraphicsPipeline::CreatePixelShaderFromMemory()でエラーが発生しました。ピクセルシェーダーの作成エラーです。")));
}

// ラスタライザステートを作成
void UGraphicsPipeline::CreateRasterizerState(ID3D11Device* pD3D11Device, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC RasterizerDesc;

	::ZeroMemory(&RasterizerDesc, sizeof(RasterizerDesc));
	RasterizerDesc.FillMode = FillMode;            // ポリゴン面描画
	RasterizerDesc.CullMode = CullMode;            // 指定の方向を向いている三角形をカリングする
	RasterizerDesc.FrontCounterClockwise = TRUE;   // 反時計回りを表面
	RasterizerDesc.DepthBias = 0;
	RasterizerDesc.DepthBiasClamp = 0;
	RasterizerDesc.SlopeScaledDepthBias = 0;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.ScissorEnable = FALSE;          // シザー矩形無効
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;

	SAFE_RELEASE(m_pRasterizerState);

	if (FAILED(hr = pD3D11Device->CreateRasterizerState(&RasterizerDesc, &m_pRasterizerState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateRasterizerState()でエラーが発生しました。ID3D11RasterizerStateインターフェースの作成エラーです。")));
}

// 深度ステンシルステートを作成
void UGraphicsPipeline::CreateDepthStencilState(ID3D11Device* pD3DDevice, BOOL DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteEnabled, D3D11_COMPARISON_FUNC DepthFunc)
{
	HRESULT hr;

	D3D11_DEPTH_STENCIL_DESC DSDesc;

	DSDesc.DepthEnable = DepthEnable;           // 深度テストを行うか
	DSDesc.DepthWriteMask = DepthWriteEnabled;  // 深度バッファへの書き込みを行うか
	DSDesc.DepthFunc = DepthFunc;
	DSDesc.StencilEnable = FALSE;

	SAFE_RELEASE(m_pDepthStencilState);

	if (FAILED(hr = pD3DDevice->CreateDepthStencilState(&DSDesc, &m_pDepthStencilState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateDepthStencilState()でエラーが発生しました。ID3D11DepthStencilStateインターフェースの作成エラーです。")));
}

// ブレンドステートを作成
void UGraphicsPipeline::CreateBlendState(ID3D11Device* pD3DDevice, UEBLEND_STATE* BlendStateType, UINT BlendStateTypeLength, BOOL AlphaToCoverageEnable)
{
	HRESULT hr;
	D3D11_BLEND_DESC BlendDesc;

	if (BlendStateTypeLength == 0 || BlendStateTypeLength >= 8)
		throw(UException(-1, _T("UGraphicsPipeline::CreateBlendState()でエラーが発生しました。仮引数が不正です。")));

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

			// 加算合成
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

			// 線形合成
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

			// 深度テスト
		case UEBLEND_STATE::DEPTH_TEST:
			BlendDesc.RenderTarget[i].BlendEnable = TRUE;
			BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_MIN;      // 最前面の深度値で上書きするために最小値を選択
			BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_MIN;
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		}
	}

	SAFE_RELEASE(m_pBlendState);

	if (FAILED(hr = pD3DDevice->CreateBlendState(&BlendDesc, &m_pBlendState)))
		throw(UException(hr, _T("UGraphicsPipeline::CreateBlendState()でエラーが発生しました。ID3D11BlendStateインターフェースの作成エラーです。")));
}

// 定数バッファを作成する
ID3D11Buffer* UGraphicsPipeline::CreateConstantBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag)
{
	HRESULT hr = E_FAIL;

	ID3D11Buffer* pBuffer = nullptr;

	// バッファー リソース。
	D3D11_BUFFER_DESC BufferDesc = { 0 };

	// サブリソース
	D3D11_SUBRESOURCE_DATA* resource = nullptr;

	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	UINT CPUAccessFlags = 0;

	__try
	{
		switch (CPUAccessFlag)
		{
			// CPUアクセスを許可しない
		case 0:
			Usage = D3D11_USAGE_DEFAULT;
			CPUAccessFlags = CPUAccessFlag;
			break;
			// CPUアクセスを許可する
		default:
			Usage = D3D11_USAGE_DYNAMIC;
			CPUAccessFlags = CPUAccessFlag;
			break;
		}

		// 初期値を設定する
		if (pData)
		{
			resource = new D3D11_SUBRESOURCE_DATA();
			resource->pSysMem = pData;
			resource->SysMemPitch = 0;
			resource->SysMemSlicePitch = 0;
		}

		// バッファの設定
		BufferDesc.ByteWidth = size;                       // バッファサイズ
		BufferDesc.Usage = Usage;                      // リソース使用法を特定する
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バッファの種類
		BufferDesc.CPUAccessFlags = CPUAccessFlags;             // CPU アクセス
		BufferDesc.MiscFlags = 0;                          // その他のフラグも設定しない

														   // バッファを作成する
		hr = pD3DDevice->CreateBuffer(&BufferDesc, resource, &pBuffer);
		if (FAILED(hr))
			throw(UException(hr, _T("UGraphicsPipeline::CreateConstantBuffer()でエラーが発生しました。バッファ作成エラーです。")));
	}
	__finally
	{
		SAFE_DELETE(resource);
	}

	return pBuffer;
}
