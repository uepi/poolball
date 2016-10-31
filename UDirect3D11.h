#ifndef UDIRECT3D11_H
#define UDIRECT3D11_H

// �C���N���[�h
#include "UCommon.h"
#include <d3d11.h>
#include "UException.h"

// xnamath�Z�p���C�u����
#include <directxmath.h>

// ���C�u�����̒ǉ�
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )  // hlsl ���R���p�C������

#define UD3D11_FORMAT        DXGI_FORMAT_R8G8B8A8_UNORM

// �[�x�X�e���V���r���[�ƃV�F�[�_�[���\�[�X�r���[�̂ǂ���Ƃ��Ă��g�p�ł���悤�ɂ��邽�ߌ^���w�肵�Ȃ�
// �t�H�[�}�b�g�ɂ��Ă�DXGI_FORMAT�Q��
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
	// DDS�t�@�C�������[�h���ăe�N�X�`���[���쐬����
	// R8G8B8A8�܂���R8G8B8X8�t�H�[�}�b�g�݂̂Ń~�b�v�}�b�v��1�̂݁A�{�����[���e�N�X�`���[�ƃL���[�u�}�b�v�ƈ��k�t�H�[�}�b�g�͖��Ή�
	void static CreateSRViewFromDDSFile(ID3D11Device* pD3DDevice, const WCHAR* szFileName, ID3D11ShaderResourceView** ppSRV);

	// �����_�[�^�[�Q�b�g�r���[���쐬����
	void static CreateRenderTargetView(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height,
		ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1,
		UINT MipmapCount = 1);

	// �{�����[���e�N�X�`���[�p�̃����_�[�^�[�Q�b�g�r���[���쐬����
	void static CreateRenderTargetViewOfVolumeTexture(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height, UINT Depth,
		ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView);

	// �����_�[�^�[�Q�b�g�r���[����V�F�[�_�[���\�[�X�r���[���쐬����
	void static CreateSRViewFromRTView(ID3D11Device* pD3DDevice, ID3D11RenderTargetView* pRTView, ID3D11ShaderResourceView** ppSRView);

	// �A���I�[�_�[�h�A�N�Z�X�r���[���쐬����
	void static CreateRenderTargetViewOfRWTexture(ID3D11Device* pD3DDevice,
		DXGI_FORMAT format, UINT Width, UINT Height,
		ID3D11UnorderedAccessView** ppUAView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1);

	// �A���I�[�_�[�h�A�N�Z�X�r���[����V�F�[�_�[���\�[�X�r���[���쐬����
	void static CreateSRViewFromUAView(ID3D11Device* pD3DDevice, ID3D11UnorderedAccessView* pUAView, ID3D11ShaderResourceView** ppSRView);

	// �[�x�X�e���V���r���[���쐬����
	void static CreateDepthStencilView(ID3D11Device* pD3DDevice,
		UINT Width, UINT Height,
		ID3D11DepthStencilView** ppDSView, ID3D11ShaderResourceView** ppSRView,
		UINT ArraySize = 1);

	// �[�x�X�e���V���r���[����V�F�[�_�[���\�[�X�r���[���쐬����
	void static CreateSRViewFromDSView(ID3D11Device* pD3DDevice, ID3D11DepthStencilView* pDSView, ID3D11ShaderResourceView** ppSRView);

	// �m�C�Y�}�b�v���쐬����
	void static CreateSRViewOfNoiseMap(ID3D11Device* pD3DDevice, UINT Width, UINT Height, DXGI_FORMAT format, ID3D11ShaderResourceView** ppSRView);
};

class UBuffers
{
private:
	UBuffers();
public:
	// �o�b�t�@���쐬���鋤�ʊ֐�
	ID3D11Buffer static * CreateBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag, D3D11_BIND_FLAG BindFlag);

	// ���_�o�b�t�@���쐬����
	inline ID3D11Buffer static * CreateVertexBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag)
	{
		return CreateBuffer(pD3DDevice, pData, size, CPUAccessFlag, D3D11_BIND_VERTEX_BUFFER);
	}

	// �C���f�b�N�X�o�b�t�@���쐬����
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
	// �T���v���[�X�e�[�g���쐬����
	ID3D11SamplerState static * CreateSamplerState(ID3D11Device* pD3DDevice,
		D3D11_TEXTURE_ADDRESS_MODE TextureAddressMode,
		D3D11_FILTER Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_COMPARISON_FUNC = D3D11_COMPARISON_ALWAYS);
};

class UDirect3D11
{
public:
	HWND                    m_hWnd;
	ID3D11Device*           m_pD3DDevice;               // Direct3D11 �f�o�C�X�B
	ID3D11DeviceContext*    m_pD3DDeviceContext;        // Direct3D11 �f�o�C�X�R���e�L�X�g�B

private:
	IDXGIAdapter*           m_pAdapter;                 // �A�_�v�^�[
	IDXGISwapChain*         m_pSwapChain;               // �X���b�v�`�F�[��
	ID3D11RenderTargetView* m_pRenderTargetView;        // �����_�����O�^�[�Q�b�g�r���[
	ID3D11DepthStencilView* m_pDepthStencilView;
	BOOL m_UseDepthBuffer, m_UseMultiSample;

	// �A�_�v�^�[���擾
	void CreateAdapter();

	// �E�B���h�E�쐬
	void InitWindow(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode);

	// �f�o�C�X�̍쐬
	void CreateDevice();

	// �X���b�v�`�F�[���̍쐬
	void CreateSwapChain(DXGI_MODE_DESC* pDisplayMode);

	// �����_�����O�^�[�Q�b�g�r���[�̍쐬
	void CreateRenderTargetView();

	// �[�x�X�e���V���r���[�̍쐬
	void CreateDepthStencilView();

	// �r���[�|�[�g�̍쐬
	void CreateViewport();

	// �E�B���h�E�A�\�V�G�[�V����
	void SetWindowAssociation();

	// �f�t�H���g�̃��X�^���C�U��ݒ�
	void SetDefaultRasterize();

	// �f�t�H���g�̐[�x�X�e���V���X�e�[�g��ݒ�
	void SetDefaultDepthStencilState();

public:
	UDirect3D11();
	virtual ~UDirect3D11();
	// ���������
	void Invalidate();

	// �f�B�X�v���C���[�h�ꗗ���쐬
	void GetDisplayMode(DXGI_MODE_DESC* pModeDesc, UINT* pNum);

	// Direct3D���쐬����
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
