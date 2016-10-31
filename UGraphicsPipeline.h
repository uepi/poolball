#ifndef UGRAPHICS_PIPELINE_H
#define UGRAPHICS_PIPELINE_H

#include "UCommon.h"
#include <d3dcompiler.h>
#include "UException.h"
#include "UDirect3D11.h"

class UGraphicsPipeline
{
private:
	// 入力レイアウト
	ID3D11InputLayout* m_pLayout;

	// 頂点シェーダー
	// とりあえずストリームアウトプットは考慮しない
	ID3D11VertexShader* m_pVertexShader;

	// ハルシェーダー
	ID3D11HullShader* m_pHullShader;

	// ドメインシェーダー
	ID3D11DomainShader* m_pDomainShader;

	// ジオメトリシェーダー
	ID3D11GeometryShader* m_pGeometryShader;

	// ピクセルシェーダー
	ID3D11PixelShader* m_pPixelShader;

	// ラスタライザステート
	ID3D11RasterizerState* m_pRasterizerState;

	// 深度ステンシルステート
	ID3D11DepthStencilState* m_pDepthStencilState;

	// ブレンドステート
	ID3D11BlendState* m_pBlendState;

public:
	enum class UEBLEND_STATE
	{
		NONE = 0,
		ADD = 1,
		ALIGNMENT = 2,
		DEPTH_TEST = 3,
	};

	enum class UEACCESS_FLAG
	{
		CPU_NONE_GPU_READWRITE = 0,   // CPUによるアクセスなし、GPUによる読込みと書込み.
		UPDATE_SUBRESOURCE = 1,       // ID3D11DeviceContext::UpdateSubresource()を使用してコピーを行う場合
		CPU_WRITE_GPU_READ = 2,       // CPUによる書込み、GPUによる読込み
	};

	UGraphicsPipeline();
	virtual ~UGraphicsPipeline();
	void Invalidate();

	// 頂点シェーダーをファイルから作成する
	void CreateVertexShaderFromFile(ID3D11Device* pD3DDevice,
		TCHAR pSrcFile[], CHAR pFunctionName[],
		D3D11_INPUT_ELEMENT_DESC pInputElementDescs[], UINT NumElements);

	// 頂点シェーダーをメモリから作成する
	void CreateVertexShaderFromMemory(ID3D11Device* pD3DDevice,
		BYTE* pShader, size_t size,
		D3D11_INPUT_ELEMENT_DESC pInputElementDescs[], UINT NumElements);

	// ハルシェーダーをファイルから作成する
	void CreateHullShaderFromFile(ID3D11Device* pD3DDevice, TCHAR pSrcFile[], CHAR pFunctionName[]);

	// ハルシェーダーをメモリから作成する
	void CreateHullShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size);

	// ドメインシェーダーをファイルから作成する
	void CreateDomainShaderFromFile(ID3D11Device* pD3DDevice, TCHAR pSrcFile[], CHAR pFunctionName[]);

	// ドメインシェーダーをメモリから作成する
	void CreateDomainShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size);

	// ジオメトリシェーダーをファイルから作成する
	void CreateGeometryShaderFromFile(ID3D11Device* pD3DDevice, TCHAR pSrcFile[], CHAR pFunctionName[]);

	// ジオメトリシェーダーをメモリから作成する
	void CreateGeometryShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size);


	// ピクセルシェーダーをファイルから作成する
	void CreatePixelShaderFromFile(ID3D11Device* pD3DDevice, TCHAR pSrcFile[], CHAR pFunctionName[]);

	// ピクセルシェーダーをメモリから作成する
	void CreatePixelShaderFromMemory(ID3D11Device* pD3DDevice, BYTE* pShader, size_t size);

	// ラスタライザステートを作成
	void CreateRasterizerState(ID3D11Device* pD3DDevice, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode = D3D11_FILL_SOLID);

	// 深度ステンシルステートを作成
	void CreateDepthStencilState(ID3D11Device* pD3DDevice, BOOL DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteEnabled, D3D11_COMPARISON_FUNC DepthFunc = D3D11_COMPARISON_LESS);

	// ブレンドステートを作成
	void CreateBlendState(ID3D11Device* pD3DDevice, UEBLEND_STATE* BlendStateType, UINT BlendStateTypeLength, BOOL AlphaToCoverageEnable = FALSE);

	// 定数バッファを作成する
	ID3D11Buffer* CreateConstantBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag);

	// 頂点シェーダーをデバイスに設定する
	inline void SetVertexShader(ID3D11DeviceContext* pD3DDeviceContext)
	{
		// 入力アセンブラー ステージを設定
		pD3DDeviceContext->IASetInputLayout(m_pLayout);
		// 頂点シェーダーをデバイスに設定する。
		pD3DDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	}

	// ハルシェーダーをデバイスに設定する
	inline void SetHullShader(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->HSSetShader(m_pHullShader, nullptr, 0);
	}

	// ドメインシェーダーをデバイスに設定する
	inline void SetDomainShader(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->DSSetShader(m_pDomainShader, nullptr, 0);
	}

	// ジオメトリシェーダーをデバイスに設定する
	inline void SetGeometryShader(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->GSSetShader(m_pGeometryShader, nullptr, 0);
	}

	// ピクセルシェーダーをデバイスに設定する
	inline void SetPixelShader(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	}

	// ラスタライザステートをデバイスに設定する
	inline void SetRasterizerState(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->RSSetState(m_pRasterizerState);
	}

	// 深度ステンシルステートをデバイスに設定する
	inline void SetDepthStencilState(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, 0);
	}

	// ブレンドステートをデバイスに設定する
	inline void SetBlendState(ID3D11DeviceContext* pD3DDeviceContext)
	{
		pD3DDeviceContext->OMSetBlendState(m_pBlendState, nullptr, 0xffffffff);
	}
};

#endif
