#include "UDirect3D11.h"

bool UMaps::IsBitMask(DDS_HEADER* pHeader, DWORD r, DWORD g, DWORD b, DWORD a)
{
	return pHeader->ddspf.dwRBitMask == r && pHeader->ddspf.dwGBitMask == g && pHeader->ddspf.dwBBitMask == b && pHeader->ddspf.dwABitMask == a;
}

void UMaps::CreateSRViewFromDDSFile(ID3D11Device* pD3DDevice, const WCHAR* szFileName, ID3D11ShaderResourceView** ppSRV)
{
	HRESULT hr = E_FAIL;
	HANDLE hFile = nullptr;
	BYTE* pHeapData = nullptr;
	ID3D11Texture2D* pTex2D = nullptr;

	__try
	{
		// �t�@�C�����J��
		hFile = ::CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			TCHAR errstr[1024];
			_stprintf_s(errstr, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�t�@�C�����J���܂���B -> %s"), szFileName);
			throw(UException(HRESULT_FROM_WIN32(GetLastError()), errstr));
		}

		// �t�@�C���̃T�C�Y���擾����
		LARGE_INTEGER FileSize = { 0 };
		::GetFileSizeEx(hFile, &FileSize);

		// 32�r�b�g�����Ȃ������𒴂���T�C�Y�̓G���[�Ƃ��ď�������
		if (FileSize.HighPart > 0)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B32�r�b�g�����Ȃ������𒴂��Ă��܂��B")));

		// �t�@�C���T�C�Y���w�b�_�[+�}�W�b�N�i���o�[�ɖ����Ȃ�
		if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(DWORD)))
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�t�@�C���T�C�Y���w�b�_�[+�}�W�b�N�i���o�[�ɖ����Ȃ��B")));

		// �w�b�_�[�̈�m��
		pHeapData = new BYTE[FileSize.LowPart];

		DWORD BytesRead = 0;

		// �t�@�C����ǂ�
		if (!::ReadFile(hFile, pHeapData, FileSize.LowPart, &BytesRead, nullptr))
			throw(UException(HRESULT_FROM_WIN32(GetLastError()), _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�t�@�C����ǂ߂܂���B")));

		// �ǂݍ��񂾃f�[�^�T�C�Y���s���̂��߃G���[
		if (BytesRead < FileSize.LowPart)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�ǂݍ��񂾃f�[�^�T�C�Y���s���B")));

		// �t�@�C���̐擪�ɂ���}�W�b�N�i���o�[�� 'DDS' �ȊO�̂Ƃ��̓t�@�C���t�H�[�}�b�g���قȂ邽�߃G���[
		DWORD dwMagicNumber = *(DWORD*)(pHeapData);
		if (dwMagicNumber != DDS_MAGIC)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�}�W�b�N�i���o�[�� 'DDS' �ȊO�B")));

		// �w�b�_�[�̈�̃|�C���^���擾
		DDS_HEADER* pHeader = reinterpret_cast<DDS_HEADER*>(pHeapData + sizeof(DWORD));

		// �T�C�Y�`�F�b�N
		if (pHeader->dwSize != sizeof(DDS_HEADER) || pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT))
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�w�b�_�̈�ɐݒ肳��Ă���T�C�Y���s���B")));

		// RGB�܂���RGBA �̔񈳏k�t�H�[�}�b�g�ȊO�͖��Ή�
		if (!(pHeader->ddspf.dwFlags & DDS_RGB))
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����BRGB�܂���RGBA�t�H�[�}�b�g�ȊO�͖��Ή��B")));

		if (pHeader->dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�{�����[���e�N�X�`���[�͖��Ή��B")));

		if (pHeader->dwCubemapFlags != 0)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�L���[�u�}�b�v�͖��Ή��B")));

		// �}�W�b�N�i���o�[ + �w�b�_�[�T�C�Y
		INT offset = sizeof(DWORD) + sizeof(DDS_HEADER);
		// �f�[�^�̈�̐擪�|�C���^���擾
		BYTE* pBitData = pHeapData + offset;

		UINT iWidth = pHeader->dwWidth;
		UINT iHeight = pHeader->dwHeight;
		UINT iMipCount = pHeader->dwMipMapCount;
		if (iMipCount == 0)
			iMipCount = 1;
		else if (iMipCount > 1)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�~�b�v�}�b�v�͖��Ή��B")));

		D3D11_TEXTURE2D_DESC desc = { 0 };

		desc.ArraySize = 1;

		if (pHeader->ddspf.dwRGBBitCount != 32)
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B32bit�t�H�[�}�b�g�̂ݑΉ��B")));

		// R8G8B8A8�܂���R8G8B8X8�̂݃T�|�[�g����
		if (IsBitMask(pHeader, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (IsBitMask(pHeader, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�s���ȃt�H�[�}�b�g�B")));

		desc.Width = iWidth;
		desc.Height = iHeight;
		desc.MipLevels = iMipCount;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;

		// �Ŋ��̃o�C�g�ɐ؂�グ
		UINT RowBytes = (iWidth * 32 + 7) / 8;

		// �f�[�^�Z�b�g
		InitData.pSysMem = (void*)pBitData;
		// 1�s������̃o�C�g��
		InitData.SysMemPitch = RowBytes;

		if (FAILED(hr = pD3DDevice->CreateTexture2D(&desc, &InitData, &pTex2D)))
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�e�N�X�`���[�̍쐬�Ɏ��s�����B")));

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		::ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = desc.MipLevels;
		if (FAILED(hr = pD3DDevice->CreateShaderResourceView(pTex2D, &SRVDesc, ppSRV)))
			throw(UException(-1, _T("LoadTextureDataFromFile()�ŃG���[���������܂����B�V�F�[�_�[���\�[�X�r���[�̍쐬�Ɏ��s�����B")));
	}
	__finally
	{
		SAFE_RELEASE(pTex2D);
		SAFE_DELETE_ARRAY(pHeapData);

		if (hFile)
			::CloseHandle(hFile);
	}
}

// �����_�[�^�[�Q�b�g�r���[���쐬����
void UMaps::CreateRenderTargetView(ID3D11Device* pD3DDevice,
	DXGI_FORMAT format, UINT Width, UINT Height,
	ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView,
	UINT ArraySize,
	UINT MipmapCount)
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pTex2D = nullptr;

	__try
	{
		D3D11_TEXTURE2D_DESC Tex2DDesc;
		::ZeroMemory(&Tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
		Tex2DDesc.ArraySize = ArraySize;
		Tex2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		if (ppSRView) Tex2DDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		Tex2DDesc.Format = format;
		Tex2DDesc.Width = Width;
		Tex2DDesc.Height = Height;
		Tex2DDesc.MipLevels = MipmapCount;    // �ő�~�b�v�}�b�v���B0�̂Ƃ��͊��S�ȃ~�b�v�}�b�v�`�F�[�����쐬����
		Tex2DDesc.SampleDesc.Count = 1;
		// 1�ȊO�̏ꍇ��ID3D11DeviceContext::GenerateMips()�ɂ��~�b�v�}�b�v�`�F�[�����쐬���邽�߂̃t���O��ݒ肷��
		if (MipmapCount != 1)
			Tex2DDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		if (FAILED(hr = pD3DDevice->CreateTexture2D(&Tex2DDesc, nullptr, &pTex2D)))
			throw(UException(hr, _T("CreateRenderTargetView()�ŃG���[���������܂����B�e�N�X�`���[���쐬�ł��܂���B")));

		if (FAILED(hr = pD3DDevice->CreateRenderTargetView(pTex2D, nullptr, ppRTView)))
			throw(UException(hr, _T("CreateRenderTargetView()�ŃG���[���������܂����B�����_�[�^�[�Q�b�g�r���[���쐬�ł��܂���B")));

		if (ppSRView != nullptr)
			UMaps::CreateSRViewFromRTView(pD3DDevice, *ppRTView, ppSRView);
	}
	__finally
	{
		SAFE_RELEASE(pTex2D);
	}
}

// �{�����[���e�N�X�`���[�p�̃����_�[�^�[�Q�b�g�r���[���쐬����
void UMaps::CreateRenderTargetViewOfVolumeTexture(ID3D11Device* pD3DDevice,
	DXGI_FORMAT format, UINT Width, UINT Height, UINT Depth,
	ID3D11RenderTargetView** ppRTView, ID3D11ShaderResourceView** ppSRView)
{
	HRESULT hr = E_FAIL;
	ID3D11Texture3D* pTex3D = nullptr;

	__try
	{
		D3D11_TEXTURE3D_DESC Tex3DDesc;
		::ZeroMemory(&Tex3DDesc, sizeof(D3D11_TEXTURE3D_DESC));
		Tex3DDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		if (ppSRView) Tex3DDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		Tex3DDesc.Usage = D3D11_USAGE_DEFAULT;
		Tex3DDesc.Format = format;
		Tex3DDesc.Width = Width;
		Tex3DDesc.Height = Height;
		Tex3DDesc.Depth = Depth;
		Tex3DDesc.MipLevels = 1;
		if (FAILED(hr = pD3DDevice->CreateTexture3D(&Tex3DDesc, nullptr, &pTex3D)))
			throw(UException(hr, _T("CreateRenderTargetViewOfVolumeTexture()�ŃG���[���������܂����B�e�N�X�`���[���쐬�ł��܂���B")));

		if (FAILED(hr = pD3DDevice->CreateRenderTargetView(pTex3D, nullptr, ppRTView)))
			throw(UException(hr, _T("CreateRenderTargetViewOfVolumeTexture()�ŃG���[���������܂����B�����_�[�^�[�Q�b�g�r���[���쐬�ł��܂���B")));

		if (ppSRView != nullptr)
			UMaps::CreateSRViewFromRTView(pD3DDevice, *ppRTView, ppSRView);
	}
	__finally
	{
		SAFE_RELEASE(pTex3D);
	}
}

// �����_�[�^�[�Q�b�g�r���[����V�F�[�_�[���\�[�X�r���[���쐬����
void UMaps::CreateSRViewFromRTView(ID3D11Device* pD3DDevice, ID3D11RenderTargetView* pRTView, ID3D11ShaderResourceView** ppSRView)
{
	HRESULT hr = E_FAIL;
	ID3D11Resource* pResource = nullptr;

	__try
	{
		// �����_�[�^�[�Q�b�g�r���[����e�N�X�`���[���擾����
		pRTView->GetResource(&pResource);

		// �V�F�[�_�[���\�[�X�r���[���쐬����
		if (FAILED(hr = pD3DDevice->CreateShaderResourceView(pResource, nullptr, ppSRView)))
			throw(UException(hr, _T("CreateSRViewFromRTView()�ŃG���[���������܂����B�V�F�[�_�[���\�[�X�r���[���쐬�ł��܂���B")));
	}
	__finally
	{
		SAFE_RELEASE(pResource);
	}
}

// �A���I�[�_�[�h�A�N�Z�X�r���[���쐬����
void UMaps::CreateRenderTargetViewOfRWTexture(ID3D11Device* pD3DDevice,
	DXGI_FORMAT format, UINT Width, UINT Height,
	ID3D11UnorderedAccessView** ppUAView, ID3D11ShaderResourceView** ppSRView,
	UINT ArraySize)
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pTex2D = nullptr;

	__try
	{
		D3D11_TEXTURE2D_DESC Tex2DDesc;
		::ZeroMemory(&Tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
		Tex2DDesc.ArraySize = ArraySize;
		Tex2DDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		if (ppSRView) Tex2DDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		Tex2DDesc.Format = format;
		Tex2DDesc.Width = Width;
		Tex2DDesc.Height = Height;
		Tex2DDesc.MipLevels = 1;
		Tex2DDesc.SampleDesc.Count = 1;
		if (FAILED(hr = pD3DDevice->CreateTexture2D(&Tex2DDesc, nullptr, &pTex2D)))
			throw(UException(hr, _T("CreateRenderTargetViewOfRWTexture()�ŃG���[���������܂����B�e�N�X�`���[���쐬�ł��܂���B")));

		if (FAILED(hr = pD3DDevice->CreateUnorderedAccessView(pTex2D, nullptr, ppUAView)))
			throw(UException(hr, _T("CreateRenderTargetViewOfRWTexture()�ŃG���[���������܂����B�A���I�[�_�[�h�A�N�Z�X�r���[���쐬�ł��܂���B")));

		if (ppSRView != nullptr)
			UMaps::CreateSRViewFromUAView(pD3DDevice, *ppUAView, ppSRView);
	}
	__finally
	{
		SAFE_RELEASE(pTex2D);
	}
}

// �A���I�[�_�[�h�A�N�Z�X�r���[����V�F�[�_�[���\�[�X�r���[���쐬����
void UMaps::CreateSRViewFromUAView(ID3D11Device* pD3DDevice, ID3D11UnorderedAccessView* pUAView, ID3D11ShaderResourceView** ppSRView)
{
	HRESULT hr = E_FAIL;
	ID3D11Resource* pResource = nullptr;

	__try
	{
		// �A���I�[�_�[�h�A�N�Z�X�r���[����e�N�X�`���[���擾����
		pUAView->GetResource(&pResource);

		// �V�F�[�_�[���\�[�X�r���[���쐬����
		if (FAILED(hr = pD3DDevice->CreateShaderResourceView(pResource, nullptr, ppSRView)))
			throw(UException(hr, _T("CreateSRViewFromUAView()�ŃG���[���������܂����B�V�F�[�_�[���\�[�X�r���[���쐬�ł��܂���B")));
	}
	__finally
	{
		SAFE_RELEASE(pResource);
	}
}

// �[�x�X�e���V���r���[���쐬����
void UMaps::CreateDepthStencilView(ID3D11Device* pD3DDevice,
	UINT Width, UINT Height,
	ID3D11DepthStencilView** ppDSView, ID3D11ShaderResourceView** ppSRView,
	UINT ArraySize)
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pTex2D = nullptr;

	__try
	{
		D3D11_TEXTURE2D_DESC Tex2DDesc;
		::ZeroMemory(&Tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
		Tex2DDesc.ArraySize = ArraySize;
		Tex2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		if (ppSRView) Tex2DDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		Tex2DDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		Tex2DDesc.Width = Width;
		Tex2DDesc.Height = Height;
		Tex2DDesc.MipLevels = 1;
		Tex2DDesc.SampleDesc.Count = 1;

		// �[�x�o�b�t�@�p�̃e�N�X�`���[�쐬
		hr = pD3DDevice->CreateTexture2D(&Tex2DDesc, nullptr, &pTex2D);
		if (FAILED(hr))
			throw(UException(hr, _T("CreateDepthStencilView()�ŃG���[���������܂����B�[�x�o�b�t�@�p�̃e�N�X�`���[�쐬�G���[�ł��B")));

		D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;

		::ZeroMemory(&DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

		// �e�N�X�`���[�쐬���Ɏw�肵���t�H�[�}�b�g�ƌ݊���������A�[�x�X�e���V���r���[�Ƃ��Ďw��ł���t�H�[�}�b�g���w�肷��
		switch (Tex2DDesc.Format)
		{
		case DXGI_FORMAT_R16_TYPELESS:
			DSVDesc.Format = DXGI_FORMAT_D16_UNORM;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			DSVDesc.Format = DSVDesc.Format;
		}

		switch (ArraySize)
		{
			// �z��Ȃ�
		case 1:
			DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			DSVDesc.Texture2D.MipSlice = 0;
			break;
			// �z�񂠂�
		default:
			DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			DSVDesc.Texture2DArray.ArraySize = ArraySize;
			DSVDesc.Texture2DArray.MipSlice = 0;
			DSVDesc.Texture2DArray.FirstArraySlice = 0;
			break;
		}
		// �[�x�X�e���V���r���[���쐬
		hr = pD3DDevice->CreateDepthStencilView(pTex2D, &DSVDesc, ppDSView);
		if (FAILED(hr))
			throw(UException(hr, _T("CreateDepthStencilView()�ŃG���[���������܂����B�[�x�X�e���V���r���[���쐬�ł��܂���B")));

		if (ppSRView != nullptr)
			UMaps::CreateSRViewFromDSView(pD3DDevice, *ppDSView, ppSRView);
	}
	__finally
	{
		SAFE_RELEASE(pTex2D);
	}
}

// �[�x�X�e���V���r���[����V�F�[�_�[���\�[�X�r���[���쐬����
void UMaps::CreateSRViewFromDSView(ID3D11Device* pD3DDevice, ID3D11DepthStencilView* pDSView, ID3D11ShaderResourceView** ppSRView)
{
	HRESULT hr = E_FAIL;
	ID3D11Resource* pResource = nullptr;

	__try
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		pDSView->GetDesc(&DSVDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		::ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		switch (DSVDesc.ViewDimension)
		{
		case D3D11_DSV_DIMENSION_TEXTURE2D:
			SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;
			break;
		case D3D11_DSV_DIMENSION_TEXTURE2DARRAY:
			SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
			SRVDesc.Texture2DArray.ArraySize = DSVDesc.Texture2DArray.ArraySize;
			SRVDesc.Texture2DArray.MipLevels = 1;
			break;
		}

		switch (DSVDesc.Format)
		{
		case DXGI_FORMAT_D16_UNORM:
			SRVDesc.Format = DXGI_FORMAT_R16_FLOAT;
			break;
		case DXGI_FORMAT_D32_FLOAT:
			SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		default:
			SRVDesc.Format = DSVDesc.Format;
		}

		// �[�x�X�e���V���r���[����e�N�X�`���[���擾����
		pDSView->GetResource(&pResource);

		// �V�F�[�_�[���\�[�X�r���[���쐬����
		if (FAILED(hr = pD3DDevice->CreateShaderResourceView(pResource, &SRVDesc, ppSRView)))
			throw(UException(hr, _T("CreateSRViewFromDSView()�ŃG���[���������܂����B�V�F�[�_�[���\�[�X�r���[���쐬�ł��܂���B")));
	}
	__finally
	{
		SAFE_RELEASE(pResource);
	}
}

// �m�C�Y�}�b�v���쐬����
void UMaps::CreateSRViewOfNoiseMap(ID3D11Device* pD3DDevice, UINT Width, UINT Height, DXGI_FORMAT format, ID3D11ShaderResourceView** ppSRView)
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pTexture2D = nullptr;
	float* pBits = nullptr;
	DWORD BitsCount = 0;

	__try
	{
		D3D11_SUBRESOURCE_DATA SubResourceData;
		::ZeroMemory(&SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));

		switch (format)
		{
		case DXGI_FORMAT_R32_FLOAT:
			BitsCount = Width * Height;
			pBits = NEW float[BitsCount];
			for (DWORD i = 0; i<BitsCount; i++)
				pBits[i] = (float)(rand() % 255) / 254.0f;
			SubResourceData.pSysMem = (LPVOID)pBits;
			SubResourceData.SysMemPitch = Width * sizeof(float);
			break;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			BitsCount = Width * Height * 4;
			pBits = NEW float[BitsCount];
			for (DWORD i = 0; i<BitsCount; i++)
				pBits[i] = (float)(rand() % 255) / 254.0f;
			SubResourceData.pSysMem = (LPVOID)pBits;
			SubResourceData.SysMemPitch = Width * sizeof(float) * 4;
			break;
		default:
			throw(UException(hr, _T("CreateSRViewOfNoiseMap()�ŃG���[���������܂����B���Ή��̃t�H�[�}�b�g���w�肳��܂����B")));
			break;
		}

		D3D11_TEXTURE2D_DESC Tex2DDesc;
		::ZeroMemory(&Tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
		Tex2DDesc.ArraySize = 1;
		Tex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		Tex2DDesc.Format = format;
		Tex2DDesc.Width = Width;
		Tex2DDesc.Height = Height;
		Tex2DDesc.MipLevels = 1;
		Tex2DDesc.SampleDesc.Count = 1;

		// �e�N�X�`���[�쐬
		if (FAILED(hr = pD3DDevice->CreateTexture2D(&Tex2DDesc, &SubResourceData, &pTexture2D)))
			throw(UException(hr, _T("CreateSRViewOfNoiseMap()�ŃG���[���������܂����B�e�N�X�`���[�̍쐬�Ɏ��s���܂���")));

		// �V�F�[�_�[���\�[�X�r���[���쐬
		if (FAILED(hr = pD3DDevice->CreateShaderResourceView(pTexture2D, nullptr, ppSRView)))
			throw(UException(hr, _T("CreateSRViewOfNoiseMap()�ŃG���[���������܂����B�V�F�[�_�[���\�[�X�r���[�̍쐬�Ɏ��s���܂���")));
	}
	__finally
	{
		SAFE_DELETE_ARRAY(pBits);
		SAFE_RELEASE(pTexture2D);
	}
}

// ���_�A�C���f�b�N�X�o�b�t�@���쐬����
ID3D11Buffer* UBuffers::CreateBuffer(ID3D11Device* pD3DDevice, void* pData, size_t size, UINT CPUAccessFlag, D3D11_BIND_FLAG BindFlag)
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
		BufferDesc.ByteWidth = size;                      // �o�b�t�@�T�C�Y
		BufferDesc.Usage = Usage;                     // ���\�[�X�g�p�@����肷��
		switch (BindFlag)
		{
		case D3D11_BIND_STREAM_OUTPUT:
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
			break;
		default:
			BufferDesc.BindFlags = BindFlag;
			break;
		}
		BufferDesc.CPUAccessFlags = CPUAccessFlags;            // CPU �A�N�Z�X
		BufferDesc.MiscFlags = 0;                         // ���̑��̃t���O���ݒ肵�Ȃ�

														  // �o�b�t�@���쐬����
		hr = pD3DDevice->CreateBuffer(&BufferDesc, resource, &pBuffer);
		if (FAILED(hr))
			throw(UException(hr, _T("CreateBuffer()�ŃG���[���������܂����B�o�b�t�@�쐬�G���[�ł��B")));
	}
	__finally
	{
		SAFE_DELETE(resource);
	}

	return pBuffer;
}

// �T���v���[�X�e�[�g���쐬����
ID3D11SamplerState* USamplers::CreateSamplerState(ID3D11Device* pD3DDevice, D3D11_TEXTURE_ADDRESS_MODE TextureAddressMode, D3D11_FILTER Filter, D3D11_COMPARISON_FUNC ComparisonFunc)
{
	HRESULT hr = E_FAIL;
	ID3D11SamplerState* pSamplerState = nullptr;

	// �T���v���[�X�e�[�g���쐬����
	D3D11_SAMPLER_DESC samplerDesc;
	::ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = Filter;                           // �T���v�����O���Ɏg�p����t�B���^�B
	samplerDesc.AddressU = TextureAddressMode;             // 0 �` 1 �͈̔͊O�ɂ��� u �e�N�X�`���[���W�̕`����@
	samplerDesc.AddressV = TextureAddressMode;             // 0 �` 1 �͈̔͊O�ɂ��� v �e�N�X�`���[���W
	samplerDesc.AddressW = TextureAddressMode;             // 0 �` 1 �͈̔͊O�ɂ��� w �e�N�X�`���[���W
	samplerDesc.MipLODBias = 0;                            // �v�Z���ꂽ�~�b�v�}�b�v ���x������̃o�C�A�X
	samplerDesc.MaxAnisotropy = 1;                         // �T���v�����O�Ɉٕ�����Ԃ��g�p���Ă���ꍇ�̌��E�l�B�L���Ȓl�� 1 �` 16 �B
	samplerDesc.ComparisonFunc = ComparisonFunc;           // ��r�I�v�V�����B
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;                                // �A�N�Z�X�\�ȃ~�b�v�}�b�v�̉����l
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // �A�N�Z�X�\�ȃ~�b�v�}�b�v�̏���l
	hr = pD3DDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	if (FAILED(hr))
		throw(UException(-1, _T("UIMesh;;CreateSamplerState()�ŃG���[���������܂����B�T���v���[�X�e�[�g���쐬�ł��܂���B")));

	return pSamplerState;
}

UDirect3D11::UDirect3D11()
{
	m_hWnd = nullptr;
	m_pD3DDevice = nullptr;
	m_pD3DDeviceContext = nullptr;
	m_pAdapter = nullptr;
	m_pSwapChain = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDepthStencilView = nullptr;
	m_UseDepthBuffer = TRUE;
	m_UseMultiSample = TRUE;
}

UDirect3D11::~UDirect3D11()
{
	Invalidate();
}

// ���������
void UDirect3D11::Invalidate()
{
	ULONG cnt;
	TCHAR s[256];

	if (m_pDepthStencilView)
	{
		cnt = m_pDepthStencilView->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������ID3D11DepthStencilView�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pDepthStencilView = nullptr;
	}


	if (m_pRenderTargetView)
	{
		cnt = m_pRenderTargetView->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������m_pRenderTargetView�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pRenderTargetView = nullptr;
	}

	if (m_pSwapChain)
	{
		// �X���b�v �`�F�[�����t���X�N���[�� ���[�h�ŉ���ł��Ȃ����Ƃ�����炵���B
		// �t���X�N���[���̏ꍇ�E�B���h�E���[�h�ɐ؂�ւ��Ă���������K�v������炵���B
		// DXGI �̊T�v
		BOOL FullScreen;
		m_pSwapChain->GetFullscreenState(&FullScreen, nullptr);
		// �t���X�N���[���̂Ƃ�
		if (FullScreen == TRUE)
			m_pSwapChain->SetFullscreenState(FALSE, nullptr);

		cnt = m_pSwapChain->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������m_pSwapChain�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pSwapChain = nullptr;
	}

	if (m_pD3DDeviceContext)
	{
		cnt = m_pD3DDeviceContext->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������m_pD3DDeviceContext�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pD3DDeviceContext = nullptr;
	}
	if (m_pD3DDevice)
	{
		cnt = m_pD3DDevice->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������m_pD3DDevice�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pD3DDevice = nullptr;
	}

	if (m_pAdapter)
	{
		cnt = m_pAdapter->Release();
		if (cnt != 0)
		{
			_stprintf_s(s, _T("������m_pAdapter�̎Q�ƃJ�E���^���s���ł��B:( %d )\n"), cnt);
			OutputDebugString(s);
		}
		m_pAdapter = nullptr;
	}
}

// �f�t�H���g�f�B�X�v���C�̃��[�h��񋓂���
void UDirect3D11::GetDisplayMode(DXGI_MODE_DESC* pModeDesc, UINT* pNum)
{
	HRESULT hr = E_FAIL;
	IDXGIOutput* pOutput = nullptr;

	__try
	{
		// �A�_�v�^�[���擾�B
		CreateAdapter();

		// �A�_�v�^�[�ɐڑ�����Ă���v���C�}���f�B�X�v���C���擾�B
		hr = m_pAdapter->EnumOutputs(0, &pOutput);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::GetDisplayMode()�ŃG���[���������܂����B�A�_�v�^�[�ɐڑ�����Ă���v���C�}���f�B�X�v���C�̎擾�Ɏ��s���܂����B")));

		// �f�B�X�v���C���[�h�̈ꗗ���擾
		hr = pOutput->GetDisplayModeList(UD3D11_FORMAT, 0, pNum, pModeDesc);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::GetDisplayMode()�ŃG���[���������܂����B�f�B�X�v���C���[�h�̈ꗗ�̎擾�Ɏ��s���܂����B")));

		if (pModeDesc == nullptr)
		{
			// 0 ���Ԃ邱�Ƃ����邩�s�������O�̂���
			if (*pNum == 0)
				throw(UException(hr, _T("UDirect3D11::GetDisplayMode()�ŃG���[���������܂����B�f�B�X�v���C���[�h���P���擾�ł��܂���B")));
		}
	}
	__finally
	{
		SAFE_RELEASE(pOutput);
	}
}

// �f�t�H���g�A�_�v�^�[�̃C���^�[�t�F�[�X���쐬����
// �A�_�v�^�[�͂P�Ɍ��肷��
void UDirect3D11::CreateAdapter()
{
	HRESULT hr = E_FAIL;
	IDXGIFactory* pFactory = nullptr;

	if (m_pAdapter == nullptr)
	{
		__try
		{
			// �t�@�N�g�����쐬����B
			hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
			if (FAILED(hr))
				throw(UException(hr, _T("UDirect3D11::CreateAdapter()�ŃG���[���������܂����B�t�@�N�g���̍쐬�G���[�ł��B")));

			// �f�t�H���g�A�_�v�^�[���擾
			hr = pFactory->EnumAdapters(0, &m_pAdapter);
			if (FAILED(hr))
				throw(UException(hr, _T("UDirect3D11::CreateAdapter()�ŃG���[���������܂����B�f�t�H���g�A�_�v�^�[�̍쐬�G���[�ł��B")));
		}
		__finally
		{
			SAFE_RELEASE(pFactory);
		}
	}
}

// �E�B���h�E�̍쐬 ����� Direct3D�̏�����
void UDirect3D11::CreateDirect3D11(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode,
	BOOL UseMultisample, BOOL UseDepthBuffer)
{
	m_UseMultiSample = UseMultisample;
	m_UseDepthBuffer = UseDepthBuffer;

	__try
	{
		// �E�B���h�E���쐬
		InitWindow(AppName, hInstance, WndProc, pDisplayMode);

		// �A�_�v�^�[���擾�B
		CreateAdapter();

		// �f�o�C�X�̍쐬
		CreateDevice();

		// �X���b�v�`�F�[���̍쐬
		CreateSwapChain(pDisplayMode);

		// �����_�����O�^�[�Q�b�g�r���[�̍쐬
		CreateRenderTargetView();

		// �[�x�X�e���V���r���[�̍쐬
		CreateDepthStencilView();

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// �r���[�|�[�g�쐬
		CreateViewport();

		// �\�����[�h�̎����؊����𖳌��ɂ���B
		// �����؊�����L���ɂ���ꍇ�͂��̊֐����R�����g�A�E�g����B
		// �������R�����g�A�E�g�����ꍇ�A�����N�����t���X�N���[�����[�h�ɂ����Ƃ����������삵�Ă���Ȃ��B
		SetWindowAssociation();

		// �f�t�H���g�̃��X�^���C�U��ݒ肷��
		SetDefaultRasterize();

		// �f�t�H���g�̐[�x�X�e���V���X�e�[�g��ݒ肷��
		SetDefaultDepthStencilState();
	}
	__finally
	{
		SAFE_RELEASE(m_pAdapter);
	}
}

// �E�B���h�E�쐬
void UDirect3D11::InitWindow(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode)
{
	// �E�B���h�E���[�h
	DWORD WindowStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU;

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = AppName;
	wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wc.hInstance = hInstance;
	::RegisterClassEx(&wc);

	RECT rc;
	::SetRect(&rc, 0, 0, pDisplayMode->Width, pDisplayMode->Height);
	// �E�B���h�E���[�h�̂Ƃ��^�C�g���o�[���l�����ĕ\���̈��ݒ肷�邽�߂ɕK�v�ȃT�C�Y���擾����
	::AdjustWindowRect(&rc, WindowStyle, FALSE);

	// �E�B���h�E���쐬
	m_hWnd = ::CreateWindow(
		wc.lpszClassName,
		wc.lpszClassName,
		WindowStyle,
		0,
		0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (m_hWnd == nullptr)
		throw(UException(-1, _T("UDirect3D11::InitWindow()�ŃG���[���������܂����B�E�B���h�E���쐬�ł��܂���B")));
}

// �f�o�C�X�̍쐬
void UDirect3D11::CreateDevice()
{
	HRESULT hr = E_FAIL;

	// �f�o�C�X�쐬���Ɏg�p����r�f�I�J�[�h�̐��\�ꗗ
	// Direct3D11.0�� SM 5.0 �̂݃T�|�[�g����
	D3D_FEATURE_LEVEL FeatureLevel[] = {
		D3D_FEATURE_LEVEL_11_0,  // Direct3D 11.0 SM 5
	};

#if defined(DEBUG) || defined(_DEBUG)
	// �f�o�b�O�R���p�C���̏ꍇ�A�f�o�b�O���C���[��L���ɂ���B
	// �\�t�g�E�F�A ���C���[
	UINT createDeviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlag = 0;
#endif

	D3D_FEATURE_LEVEL level;
	// �f�o�C�X���쐬����B
	hr = D3D11CreateDevice(
		m_pAdapter,                 // �g�p����A�_�v�^�[��ݒ�Bnullptr�̏ꍇ�̓f�t�H���g�̃A�_�v�^�[�B
		D3D_DRIVER_TYPE_UNKNOWN,    // D3D_DRIVER_TYPE�̂����ꂩ�B�h���C�o�[�̎�ށBpAdapter�� nullptr �ȊO�̏ꍇ�́AD3D_DRIVER_TYPE_UNKNOWN���w�肷��B
		nullptr,                    // �\�t�g�E�F�A���X�^���C�U����������DLL�ւ̃n���h���BD3D_DRIVER_TYPE �� D3D_DRIVER_TYPE_SOFTWARE �ɐݒ肵�Ă���ꍇ�� nullptr �ɂł��Ȃ��B
		createDeviceFlag,           // D3D11_CREATE_DEVICE_FLAG�̑g�ݍ��킹�B�f�o�C�X���쐬���Ɏg�p�����p�����[�^�B
		FeatureLevel,               // D3D_FEATURE_LEVEL�̃|�C���^
		1,                          // D3D_FEATURE_LEVEL�z��̗v�f��
		D3D11_SDK_VERSION,          // DirectX SDK�̃o�[�W�����B���̒l�͌Œ�B
		&m_pD3DDevice,              // ���������ꂽ�f�o�C�X
		&level,                     // �̗p���ꂽ�t�B�[�`���[���x��
		&m_pD3DDeviceContext        // ���������ꂽ�f�o�C�X�R���e�L�X�g
	);
	if (FAILED(hr))
		throw(UException(hr, _T("UDirect3D11::CreateDevice()�ŃG���[���������܂����B�f�o�C�X�̍쐬�G���[�ł��B")));

	// �A�_�v�^�[�̏����擾
	DXGI_ADAPTER_DESC desc;
	m_pAdapter->GetDesc(&desc);

	// DirectX�̃o�[�W�����A�V�F�[�_���f���̃o�[�W�����ƍ̗p�����A�_�v�^�[�̖��O���f�o�b�N�E�B���h�E�ɏo��
	TCHAR s[256];
	_stprintf_s(s, _T("�������f�o�C�X�̍쐬�ɐ����I�I Direct3D(11.0) SM(5)�@%s\n"), desc.Description);
	OutputDebugString(s);
}

// �X���b�v�`�F�[���̍쐬
void UDirect3D11::CreateSwapChain(DXGI_MODE_DESC* pDisplayMode)
{
	HRESULT hr = E_FAIL;
	TCHAR s[256];
	IDXGIFactory* pFactory = nullptr;
	DXGI_SWAP_CHAIN_DESC sd;

	__try
	{
		::ZeroMemory(&sd, sizeof(sd));
		::CopyMemory(&(sd.BufferDesc), pDisplayMode, sizeof(DXGI_MODE_DESC));

		sd.BufferDesc.Format = UD3D11_FORMAT;                            // �o�b�N�o�b�t�@�t�H�[�}�b�g
																		 // �o�b�N�o�b�t�@�̃T�[�t�F�X�g�p�@����� CPU �A�N�Z�X �I�v�V������\�� DXGI_USAGE �񋓌^�̃����o�[
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;                // �o�b�N�o�b�t�@�Ƀ����_�����O�\�ɂ���B
																		 // �X���b�v �`�F�[���̃o�b�t�@�[����\���l�B
		sd.BufferCount = 1;
		// �o�̓E�B���h�E�ւ� HWND �n���h���B���̃����o�[�� nullptr �ɂ͂ł��Ȃ��B
		sd.OutputWindow = m_hWnd;
		// �o�͂��E�B���h�E ���[�h�̏ꍇ�� TRUE�B����ȊO�̏ꍇ�� FALSE�B
		// �����N�����̕\�����[�h�̓E�B���h�E���[�h�ɌŒ�Ƃ���B
		sd.Windowed = TRUE;
		// �T�[�t�F�X�̃X���b�v������Ƀo�b�N�o�b�t�@�̓��e��ێ����邩�B
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;                        // �o�b�N�o�b�t�@�̓��e��ێ����Ȃ��B
																		 // �X���b�v �`�F�[���̓���̃I�v�V�����B
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;               // �t���X�N���[�����[�h�ɐ؂�ւ���ƁA�A�v���P�[�V���� �E�B���h�E�̃T�C�Y�ɍ��킹�ĉ𑜓x���ύX�����B

																		 // �t�@�N�g���[���쐬����
																		 // CreateDXGIFactory�ō쐬�����t�@�N�g���[���g�p����Ǝ��s�ł��邪���[�j���O�G���[�ɂȂ�̂� IDXGIAdapter ����쐬����B
		hr = m_pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateSwapChain()�ŃG���[���������܂����B�t�@�N�g���[�̎擾�G���[�ł��B")));

		// �}���`�T���v�����O�̐ݒ�
		// �}���`�T���v�����g�p���Ȃ��ꍇ�̐ݒ�
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		int startMultisample;

		// �T���v�����O�����ő�̂��̂��̗p����
		if (m_UseMultiSample == TRUE)
			startMultisample = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
		// �}���`�T���v���𖳌��ɂ���B
		else
			startMultisample = 1;

		UINT Quality = 0;
		for (int i = startMultisample; i >= 0; i--)
		{
			// �T�|�[�g����N�H���e�B���x���̍ő�l���擾����
			hr = m_pD3DDevice->CheckMultisampleQualityLevels(pDisplayMode->Format, (UINT)i, &Quality);
			if (FAILED(hr))
				throw(UException(hr, _T("UDirect3D11::CreateSwapChain()�ŃG���[���������܂����B�T�|�[�g����N�H���e�B���x���̍ő�l�̎擾�G���[�ł��B")));

			// 0 �ȊO�̂Ƃ��t�H�[�}�b�g�ƃT���v�����O���̑g�ݍ��킹���T�|�[�g����
			// �I�����C���w���v�ł�CheckMultisampleQualityLevels�̖߂�l�� 0 �̂Ƃ��T�|�[�g����Ȃ��Ƃ�����Ă��邪
			// pNumQualityLevels �̖߂�l�� 0 ���Ŕ��f����B
			// Direct3D 10 �� �̃I�����C���w���v�ɂ͂���������Ă���̂ŁA�I�����C���w���v�̊ԈႢ�B
			if (Quality != 0)
			{
				sd.SampleDesc.Count = i;
				sd.SampleDesc.Quality = Quality - 1;

				// �X���b�v�`�F�[�����쐬����B
				hr = pFactory->CreateSwapChain(m_pD3DDevice, &sd, &m_pSwapChain);
				_stprintf_s(s, _T("�������}���`�T���v���FCount(%d) Quality(%d)"), sd.SampleDesc.Count, sd.SampleDesc.Quality);
				if (SUCCEEDED(hr))
				{
					if (sd.SampleDesc.Count == 1)
						m_UseMultiSample = FALSE;

					_tcscat_s(s, _T("�@�ˁ@OK\n"));
					OutputDebugString(s);
					break;
				}
				else
				{
					_tcscat_s(s, _T("�@�ˁ@NG\n"));
					OutputDebugString(s);
				}
			}
		}
		if (m_pSwapChain == nullptr)
			throw(UException(-1, _T("UDirect3D11::CreateSwapChain()�ŃG���[���������܂����B�X���b�v�`�F�[�����쐬�ł��܂���ł���")));

		_stprintf_s(s, _T("�������f�B�X�v���C���[�h�F�𑜓x( %d : %d )�@���t���b�V�����[�g( %d / %d )\n"),
			pDisplayMode->Width, pDisplayMode->Height,
			pDisplayMode->RefreshRate.Denominator, pDisplayMode->RefreshRate.Numerator);
		OutputDebugString(s);
	}
	__finally
	{
		SAFE_RELEASE(pFactory);
	}
}

// �����_�����O�^�[�Q�b�g�r���[�̍쐬
void UDirect3D11::CreateRenderTargetView()
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pBackBuffer = nullptr;

	__try
	{
		// �X���b�v�`�F�[������o�b�N�o�b�t�@���擾����
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateRenderTargetView()�ŃG���[���������܂����B�X���b�v�`�F�[������o�b�N�o�b�t�@�̎擾�G���[�ł��B")));

		// �����_�����O�^�[�Q�b�g�r���[�̍쐬
		hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateRenderTargetView()�ŃG���[���������܂����B�����_�����O�^�[�Q�b�g�r���[�̍쐬�G���[�ł��B")));
	}
	__finally
	{
		SAFE_RELEASE(pBackBuffer);
	}
}

// �[�x�X�e���V���r���[�̍쐬
void UDirect3D11::CreateDepthStencilView()
{
	HRESULT hr = E_FAIL;
	ID3D11Texture2D* pBackBuffer = nullptr;
	ID3D11Texture2D* pDepthBuffer = nullptr;
	D3D11_TEXTURE2D_DESC descDS;
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	DXGI_SWAP_CHAIN_DESC chainDesc;

	if (m_UseDepthBuffer == FALSE)
		return;

	__try
	{
		// �X���b�v�`�F�[���̐ݒ���擾����
		hr = m_pSwapChain->GetDesc(&chainDesc);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateDepthStencilView()�ŃG���[���������܂����B�X���b�v�`�F�[���̐ݒ�̎擾�G���[�ł��B")));

		::ZeroMemory(&descDS, sizeof(D3D11_TEXTURE2D_DESC));
		descDS.Width = chainDesc.BufferDesc.Width;   // �o�b�N�o�b�t�@�Ɠ����ɂ���B
		descDS.Height = chainDesc.BufferDesc.Height;  // �o�b�N�o�b�t�@�Ɠ����ɂ���B
		descDS.MipLevels = 1;                            // �~�b�v�}�b�v���쐬���Ȃ�
		descDS.ArraySize = 1;                            // �e�N�X�`���[�̔z��
		descDS.Format = UD3D11_DEPTH_FORMAT;          // �t�H�[�}�b�g
		descDS.SampleDesc.Count = chainDesc.SampleDesc.Count;   // �o�b�N�o�b�t�@�Ɠ����ɂ���B
		descDS.SampleDesc.Quality = chainDesc.SampleDesc.Quality; // �o�b�N�o�b�t�@�Ɠ����ɂ���B
		descDS.Usage = D3D11_USAGE_DEFAULT;          // GPU �ɂ��ǂݎ�肨��я������݃A�N�Z�X��K�v�Ƃ��郊�\�[�X�B
		descDS.BindFlags = D3D11_BIND_DEPTH_STENCIL |    // �[�x�X�e���V���o�b�t�@�Ƃ��č쐬����
			D3D11_BIND_SHADER_RESOURCE;   // �V�F�[�_�[���\�[�X�r���[�Ƃ��č쐬����
		descDS.CPUAccessFlags = 0;                            // CPU �A�N�Z�X���s�v�B
		descDS.MiscFlags = 0;                            // ���̑��̃t���O���ݒ肵�Ȃ��B
														 // �[�x�o�b�t�@�p�̃e�N�X�`���[�쐬
		hr = m_pD3DDevice->CreateTexture2D(&descDS, nullptr, &pDepthBuffer);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateDepthStencilView()�ŃG���[���������܂����B�[�x�o�b�t�@�p�̃e�N�X�`���[�쐬�G���[�ł��B")));

		::ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

		// �e�N�X�`���[�쐬���Ɏw�肵���t�H�[�}�b�g�ƌ݊���������A�[�x�X�e���V���r���[�Ƃ��Ďw��ł���t�H�[�}�b�g���w�肷��
		switch (descDS.Format)
		{
			// 8�r�b�g�t�H�[�}�b�g�͎g�p�ł��Ȃ��H
		case DXGI_FORMAT_R8_TYPELESS:
			descDSV.Format = DXGI_FORMAT_R8_UNORM;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			descDSV.Format = DXGI_FORMAT_D16_UNORM;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			descDSV.Format = descDS.Format;
		}

		// �}���`�T���v�����g�p���Ă���ꍇ
		if (chainDesc.SampleDesc.Count > 1)
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		// �}���`�T���v�����g�p���Ă��Ȃ��ꍇ
		else
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		descDSV.Texture2D.MipSlice = 0;

		// �[�x�X�e���V���r���[���쐬
		hr = m_pD3DDevice->CreateDepthStencilView(pDepthBuffer, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::CreateDepthStencilView()�ŃG���[���������܂����B�[�x�X�e���V���r���[�̍쐬�G���[�ł��B")));
	}
	__finally
	{
		SAFE_RELEASE(pDepthBuffer);
		SAFE_RELEASE(pBackBuffer);
	}
}

// �r���[�|�[�g�̍쐬
void UDirect3D11::CreateViewport()
{
	HRESULT hr = E_FAIL;

	DXGI_SWAP_CHAIN_DESC chainDesc;
	// �X���b�v�`�F�[���̐ݒ���擾����
	hr = m_pSwapChain->GetDesc(&chainDesc);
	if (FAILED(hr))
		throw(UException(hr, _T("UDirect3D11::CreateViewport()�ŃG���[���������܂����B�X���b�v�`�F�[���̐ݒ�̎擾�G���[�ł��B")));

	D3D11_VIEWPORT Viewport[1];
	Viewport[0].TopLeftX = 0;
	Viewport[0].TopLeftY = 0;
	Viewport[0].Width = (FLOAT)chainDesc.BufferDesc.Width;
	Viewport[0].Height = (FLOAT)chainDesc.BufferDesc.Height;
	Viewport[0].MinDepth = 0.0f;
	Viewport[0].MaxDepth = 1.0f;
	m_pD3DDeviceContext->RSSetViewports(1, Viewport);
}

// �E�B���h�E�A�\�V�G�[�V����
void UDirect3D11::SetWindowAssociation()
{
	HRESULT hr = E_FAIL;
	IDXGIFactory* pFactory = nullptr;

	__try
	{
		// �t�@�N�g���[���쐬����
		// CreateDXGIFactory�ō쐬�����t�@�N�g���[���g�p����Ǝ��s�ł��邪���[�j���O�G���[�ɂȂ�̂� IDXGIAdapter ����쐬����B
		hr = m_pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::SetWindowAssociation()�ŃG���[���������܂����B�t�@�N�g���[�̎擾�G���[�ł��B")));

		// �\�����[�h�̎����؊����𖳌��ɂ���B
		hr = pFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::SetWindowAssociation()�ŃG���[���������܂����B�E�B���h�E�A�\�V�G�[�V�����̍쐬�G���[�ł��B")));
	}
	__finally
	{
		SAFE_RELEASE(pFactory);
	}
}

// �f�t�H���g�̃��X�^���C�U��ݒ�
void UDirect3D11::SetDefaultRasterize()
{
	HRESULT hr = E_FAIL;
	ID3D11RasterizerState* g_pRasterState = nullptr;
	D3D11_RASTERIZER_DESC rsState;

	__try
	{
		rsState.FillMode = D3D11_FILL_SOLID;    // �|���S���ʕ`��
		rsState.CullMode = D3D11_CULL_BACK;     // ���ʂ�`�悵�Ȃ�
		rsState.FrontCounterClockwise = TRUE;   // �����v����\��
		rsState.DepthBias = 0;
		rsState.DepthBiasClamp = 0;
		rsState.SlopeScaledDepthBias = 0;
		rsState.DepthClipEnable = TRUE;
		rsState.ScissorEnable = FALSE;          // �V�U�[��`����

												// �X���b�v�`�F�[���̃}���`�T���v�����O�̐ݒ�ɂ��킹��
		DXGI_SWAP_CHAIN_DESC swapDesc;
		m_pSwapChain->GetDesc(&swapDesc);
		if (swapDesc.SampleDesc.Count != 1)
			rsState.MultisampleEnable = TRUE;
		else
			rsState.MultisampleEnable = FALSE;

		rsState.AntialiasedLineEnable = FALSE;
		hr = m_pD3DDevice->CreateRasterizerState(&rsState, &g_pRasterState);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::SetDefaultRasterize()�ŃG���[���������܂����B���X�^���C�U�X�e�[�g�̍쐬�G���[�ł��B")));

		m_pD3DDeviceContext->RSSetState(g_pRasterState);
	}
	__finally
	{
		SAFE_RELEASE(g_pRasterState);
	}
}

// �f�t�H���g�̐[�x�X�e���V���X�e�[�g��ݒ�
void UDirect3D11::SetDefaultDepthStencilState()
{
	HRESULT hr = E_FAIL;
	ID3D11DepthStencilState* pDepthStencilState = nullptr;
	D3D11_DEPTH_STENCIL_DESC dsState;

	__try
	{
		::ZeroMemory(&dsState, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dsState.DepthEnable = TRUE;
		dsState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsState.DepthFunc = D3D11_COMPARISON_LESS;
		dsState.StencilEnable = FALSE;

		hr = m_pD3DDevice->CreateDepthStencilState(&dsState, &pDepthStencilState);
		if (FAILED(hr))
			throw(UException(hr, _T("UDirect3D11::SetDefaultDepthStencilState()�ŃG���[���������܂����B�[�x�X�e���V���X�e�[�g�̍쐬�G���[�ł��B")));

		m_pD3DDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
	}
	__finally
	{
		SAFE_RELEASE(pDepthStencilState);
	}
}

HRESULT UDirect3D11::Present(UINT SyncInterval, UINT Flags)
{
	HRESULT hr = E_FAIL;
	if (FAILED(hr = m_pSwapChain->Present(SyncInterval, Flags)))
		throw(UException(hr, _T("UDirect3D11::Present()�ŃG���[���������܂����B")));

	return hr;
}
