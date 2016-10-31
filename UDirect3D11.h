#ifndef UDIRECT3D11_H
#define UDIRECT3D11_H

// インクルード
#include "UCommon.h"
#include <d3d11.h>
#include "UException.h"

// xnamath算術ライブラリ
#include <directxmath.h>

// ライブラリの追加
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )  // hlsl をコンパイルする

#define UD3D11_FORMAT        DXGI_FORMAT_R8G8B8A8_UNORM

// 深度ステンシルビューとシェーダーリソースビューのどちらとしても使用できるようにするため型を指定しない
// フォーマットについてはDXGI_FORMAT参照
#define UD3D11_DEPTH_FORMAT  DXGI_FORMAT_R32_TYPELESS

#define DDS_MAGIC 0x20534444 // "DDS "

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

class UMaps
{
private:
	struct DDS_PIXELFORMAT
	{
		DWORD dwSize;
		DWORD dwFlags;
		DWORD dwFourCC;
		DWORD dwRGBBitCount;
		DWORD dwRBitMask;
		DWORD dwGBitMask;
		DWORD dwBBitMask;
		DWORD dwABitMask;
	};

	typedef struct
	{
		DWORD dwSize;
		DWORD dwHeaderFlags;
		DWORD dwHeight;
		DWORD dwWidth;
		DWORD dwPitchOrLinearSize;
		DWORD dwDepth;
		DWORD dwMipMapCount;
		DWORD dwReserved1[11];
		DDS_PIXELFORMAT ddspf;
		DWORD dwSurfaceFlags;
		DWORD dwCubemapFlags;
		DWORD dwReserved2[3];
	} DDS_HEADER;

	UMaps();
	bool static IsBitMask(DDS_HEADER* pHeader, DWORD r, DWORD g, DWORD b, DWORD a);

public:
	// DDSファイルをロードしてテクスチャーを作成する
	// R8G8B8A8またはR8G8B8X8フォーマットのみでミップマップは1のみ、ボリュームテクスチャーとキューブマップと圧縮フォーマットは未対応
	void static CreateSRViewFromDDSFile(ID3D11Device* pD3DDevice, const WCHAR* szFileName, ID3D11ShaderResourceView** ppSRV);

	// レンダーターゲットビューを作成する
	void static CreateRenderTargetView(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height,
		ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1,
		UINT MipmapCount = 1);

	// ボリュームテクスチャー用のレンダーターゲットビューを作成する
	void static CreateRenderTargetViewOfVolumeTexture(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height, UINT Depth,
		ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView);

	// レンダーターゲットビューからシェーダーリソースビューを作成する
	void static CreateSRViewFromRTView(ID3D11Device* pD3DDevice, ID3D11RenderTargetView* pRTView, ID3D11ShaderResourceView** ppSRView);

	// アンオーダードアクセスビューを作成する
	void static CreateRenderTargetViewOfRWTexture(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height,
		ID3D11UnorderedAccessView** ppUAView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1);

	// アンオーダードアクセスビューからシェーダーリソースビューを作成する
	void static CreateSRViewFromUAView(ID3D11Device* pD3DDevice, ID3D11UnorderedAccessView* pUAView, ID3D11ShaderResourceView** ppSRView);

	// 深度ステンシルビューを作成する
	void static CreateDepthStencilView(ID3D11Device* pD3DDevice,
		UINT Width, UINT Height,
		ID3D11DepthStencilView** ppDSView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1);

	// 深度ステンシルビューからシェーダーリソースビューを作成する
	void static CreateSRViewFromDSView(ID3D11Device* pD3DDevice, ID3D11DepthStencilView* pDSView, ID3D11ShaderResourceView** ppSRView);

	// ノイズマップを作成する
	void static CreateSRViewOfNoiseMap(ID3D11Device* pD3DDevice, UINT Width, UINT Height, DXGI_FORMAT format, ID3D11ShaderResourceView** ppSRView);
};

class UBuffers
{
private:
	UBuffers();
public:
	// バッファを作成する共通関数
	ID3D11Buffer static * CreateBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag, D3D11_BIND_FLAG BindFlag);

	// 頂点バッファを作成する
	inline ID3D11Buffer static * CreateVertexBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag)
	{
		return CreateBuffer(pD3DDevice, pData, size, CPUAccessFlag, D3D11_BIND_VERTEX_BUFFER);
	}

	// インデックスバッファを作成する
	inline ID3D11Buffer static * CreateIndexBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag)
	{
		return CreateBuffer(pD3DDevice, pData, size, CPUAccessFlag, D3D11_BIND_INDEX_BUFFER);
	}
};

class USamplers
{
private:
	USamplers();
public:
	// サンプラーステートを作成する
	ID3D11SamplerState static * CreateSamplerState(ID3D11Device* pD3DDevice,
		D3D11_TEXTURE_ADDRESS_MODE TextureAddressMode,
		D3D11_FILTER Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_COMPARISON_FUNC = D3D11_COMPARISON_ALWAYS);
};

class UDirect3D11
{
public:
	HWND                    m_hWnd;
	ID3D11Device*           m_pD3DDevice;               // Direct3D11 デバイス。
	ID3D11DeviceContext*    m_pD3DDeviceContext;        // Direct3D11 デバイスコンテキスト。

private:
	IDXGIAdapter*           m_pAdapter;                 // アダプター
	IDXGISwapChain*         m_pSwapChain;               // スワップチェーン
	ID3D11RenderTargetView* m_pRenderTargetView;        // レンダリングターゲットビュー
	ID3D11DepthStencilView* m_pDepthStencilView;
	BOOL m_UseDepthBuffer, m_UseMultiSample;

	// アダプターを取得
	void CreateAdapter();

	// ウィンドウ作成
	void InitWindow(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode);

	// デバイスの作成
	void CreateDevice();

	// スワップチェーンの作成
	void CreateSwapChain(DXGI_MODE_DESC* pDisplayMode);

	// レンダリングターゲットビューの作成
	void CreateRenderTargetView();

	// 深度ステンシルビューの作成
	void CreateDepthStencilView();

	// ビューポートの作成
	void CreateViewport();

	// ウィンドウアソシエーション
	void SetWindowAssociation();

	// デフォルトのラスタライザを設定
	void SetDefaultRasterize();

	// デフォルトの深度ステンシルステートを設定
	void SetDefaultDepthStencilState();

public:
	UDirect3D11();
	virtual ~UDirect3D11();
	// メモリ解放
	void Invalidate();

	// ディスプレイモード一覧を作成
	void GetDisplayMode(DXGI_MODE_DESC* pModeDesc, UINT* pNum);

	// Direct3Dを作成する
	void CreateDirect3D11(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode,
		BOOL UseMultisample, BOOL UseDepthBuffer);

	inline void ClearBackBuffer(const FLOAT ColorRGBA[4])
	{
		m_pD3DDeviceContext->ClearRenderTargetView(m_pRenderTargetView, ColorRGBA);
	}
	inline void ClearDepthStencilView(UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
	{
		if (m_pDepthStencilView) m_pD3DDeviceContext->ClearDepthStencilView(m_pDepthStencilView, ClearFlags, Depth, Stencil);
	}

	HRESULT Present(UINT SyncInterval, UINT Flags);
};

#endif
