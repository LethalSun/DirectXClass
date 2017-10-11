#define main2

#ifndef main2


#include <windows.h>
#include <time.h>
//헤더와 기본 전역 변수
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <DirectXMath.h>

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11d.lib")

HWND  g_hWnd = NULL;
IDXGISwapChain * g_pSwapChain = NULL;
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;

D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;

//main2추가
ID3D11VertexShader* g_pVertexShader = NULL;
ID3D11InputLayout* g_pVertexLayout = NULL;
ID3D11Buffer* g_pVertexBuffer = NULL;
using namespace DirectX;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11Buffer* g_pIndexBuffer;

struct MyVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};
/*
struct MyVertex
{
	XMFLOAT3 pos;

};
*/


HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	//Flag 설정
	UINT createDeviceFlag = 0;
#ifdef _DEBUG
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;

	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		createDeviceFlag,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		&g_featureLevel,
		&g_pImmediateContext
	);

	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		return hr;
	}

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	return hr;
}

void Render()
{
	float clearColor[4] = { 0.45f,0.f,0.7f,1.0f };

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->DrawIndexed(6, 0,0);

	g_pSwapChain->Present(0, 0);
}
void CleanupDevice()
{
	if (g_pIndexBuffer)g_pIndexBuffer->Release();

	if (g_pVertexBuffer)g_pVertexBuffer->Release();

	if (g_pVertexLayout)g_pVertexLayout->Release();

	if (g_pVertexShader)g_pVertexShader->Release();

	if (g_pPixelShader)g_pPixelShader->Release();

	if (g_pRenderTargetView)
	{
		g_pRenderTargetView->Release();
	}

	if (g_pImmediateContext)
	{
		g_pImmediateContext->ClearState();
	}

	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
	}

	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
	}
}

//TODO: hr로 안전 코드를 만들어야 한다
void CreateShader()
{

	ID3DBlob *pErrorBlob = NULL;
	ID3DBlob *pVSBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		"VS", "vs_5_0",
		0, 0, 0,
		&pVSBlob, &pErrorBlob, 0);

	hr = g_pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		0, &g_pVertexShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, 
		pVSBlob->GetBufferPointer(), 
		pVSBlob->GetBufferSize(), 
		&g_pVertexLayout);

	pVSBlob->Release();

	ID3DBlob *pPSBlob = NULL;
	D3DX11CompileFromFile(L"MyShader.fx", 0, 0,
		"PS", "ps_5_0",
		0, 0, 0,
		&pPSBlob, &pErrorBlob, 0);

	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPixelShader);

	pPSBlob->Release();
}

void CreateVertexBuffer()
{
	MyVertex vertice[]=
	{
		{ XMFLOAT3(-0.5f,0.5f,1.f),XMFLOAT4(1.f,0.f,0.f,1.f) },
		{ XMFLOAT3(0.5f,0.5f,1.f),XMFLOAT4(0.f,1.f,0.f,1.f) },
		{ XMFLOAT3 (0.5f,-0.5f,1.f),XMFLOAT4(0.f,0.f,1.f,1.f) },
		{ XMFLOAT3(-0.5f,-0.5f,1.f),XMFLOAT4(1.f,1.f,0.f,1.f) }
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertice);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertice;
	g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);

}

void CreateIndexBuffer()
{
	UINT indices[] =
	{
		0,1,2,
		0,2,3
	};

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(iinitData));
	iinitData.pSysMem = indices;
	g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer);
}

#define szWindowClass	TEXT("First")
#define szTitle			TEXT("First App")

LRESULT CALLBACK WndProc(HWND hWnd
	, UINT message
	, WPARAM wParam
	, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszCmdParam,
	int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;

	if (!RegisterClassEx(&wcex))
		return 0;

	HWND	hWnd = CreateWindowEx(WS_EX_APPWINDOW
		, szWindowClass
		, szTitle
		, WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, 800
		, 600
		, NULL
		, NULL
		, hInstance
		, NULL);

	if (!hWnd)
		return 0;

	ShowWindow(hWnd, nCmdShow);

	g_hWnd = hWnd;
	InitDevice();
	CreateShader();
	CreateVertexBuffer();
	CreateIndexBuffer();

	MSG			msg;

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}

	}
	CleanupDevice();
	return (int)msg.wParam;
}

// 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND hWnd
	, UINT message
	, WPARAM wParam
	, LPARAM lParam)
{
	HDC	hdc;
	PAINTSTRUCT	ps;

	switch (message)
	{
	case WM_CREATE:

		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif // !main2