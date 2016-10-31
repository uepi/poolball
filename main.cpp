#include <d3d11.h>
#include<Windows.h>
#include <tchar.h>
#include <iostream>
#include<vector>
#include <directxmath.h>
using namespace std;

#include "UDirect3D11.h"
#include "UCommon.h"
#include "pmd.h" //pmdローダー
#include "BasicVS.hlsl"//頂点シェーダー

//定数定義
#define WINDOW_WIDTH 1920 //ウィンドウ幅
#define WINDOW_HEIGHT 1080 //ウィンドウ高さ

//グローバル変数
HWND hWnd = NULL;
ID3D11Device* Device = NULL;
ID3D11DeviceContext* DeviceContext = NULL;
IDXGISwapChain* SwapChain = NULL;
ID3D11RenderTargetView* RenderTargetView = NULL;

//ダイレクト3Dの初期化関数
HRESULT InitD3D(HWND hWnd)
{
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, 0,
		&FeatureLevel, 1,
		D3D11_SDK_VERSION, &sd, &SwapChain, &Device, NULL, &DeviceContext)))
	{
		return FALSE;
	}
	//レンダーターゲットビューの作成
	ID3D11Texture2D *BackBuffer;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
	Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
	BackBuffer->Release();
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, NULL);
	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);

	return S_OK;
}




//メイン関数
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	//ウインドウクラスの登録
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Window1", NULL };
	RegisterClassEx(&wc);
	//タイトルバーとウインドウ枠の分を含めてウインドウサイズを設定
	RECT rect;
	SetRect(&rect, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	rect.right = rect.right - rect.left;
	rect.bottom = rect.bottom - rect.top;
	rect.top = 0;
	rect.left = 0;
	//ウインドウの生成
	hWnd = CreateWindow(L"Window1", L"Hello DirectX11 World !!",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
		NULL, NULL, wc.hInstance, NULL);

	//ポリゴン頂点構造体
	struct Vertex3D {
		float pos[3];	//x-y-z
		float col[4];	//r-g-b-a
		float tex[2];
	
	};
	//pmdクラスを生成する。その際にコンストラクタにファイル名を教える
	pmd *modeldata;
	modeldata = new pmd("board.pmd");

	//頂点データを取得

	vector<Vertex3D> TYOUTEN;
	//頂点レイアウト
	//５番目のパラメータは先頭からのバイト数なので，COLORにはPOSITIONのfloat型4バイト×3を記述
	D3D11_INPUT_ELEMENT_DESC hInElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	for (int i = 0; i < modeldata->vert_count; ++i) {
		Vertex3D tmp;

		tmp.pos[0] = modeldata->vertex[i].pos[0];
		tmp.pos[1] = modeldata->vertex[i].pos[1];
		tmp.pos[2] = modeldata->vertex[i].pos[2];
		tmp.col[0] = 0.0f;
		tmp.col[1] = 0.0f;
		tmp.col[2] = 0.5f;
		tmp.col[3] = 1.0f;
		tmp.tex[0] = 0.0f;
		tmp.tex[1] = 1.0f;

		TYOUTEN.push_back(tmp);
	}

	//インデックスデータを取得
	int INDEXSU = modeldata->face_vert_count;

	unsigned short *hIndexData;
	hIndexData = new unsigned short[INDEXSU];

	for (int i = 0; i < INDEXSU; i++) {
		hIndexData[i] = modeldata->face_vert_index[i];
	}

	
	
    
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	//Direct3D初期化
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		//ウインドウ表示
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{

				
			
			}
		}
	}
	End:
	//終了
	return 0;

}