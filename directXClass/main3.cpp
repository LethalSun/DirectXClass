#define main33

#ifndef main3


#include <windows.h>
#include <time.h>
//����� �⺻ ���� ����
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <dxerr.h>
//DX12�� DirectXMath.h �� ����ؾ� �Ѵ�.

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

//main2�߰�
ID3D11VertexShader* g_pVertexShader = NULL;
ID3D11InputLayout* g_pVertexLayout = NULL;
ID3D11Buffer* g_pVertexBuffer = NULL;// Vertex Buffer, Index Buffer.. ��� ���� ���� �������̽�
using namespace DirectX;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11Buffer* g_pIndexBuffer;

//main3 �߰�
XMMATRIX g_World;
XMMATRIX g_World2;
XMMATRIX g_View;
XMMATRIX g_Projection;
ID3D11Buffer* g_PConstantBuffer;
ID3D11Texture2D* g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;

struct MyVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct ConstantBuffer
{
	XMMATRIX wvp;
};
/*
struct MyVertex
{
XMFLOAT3 pos;

};
*/
void InitMatrix();
void CreateDepthStencilTexture();
HRESULT InitDevice()
{

	//Create Device Setting
	HRESULT hr = S_OK;

	//Flag ����
	UINT createDeviceFlag = 0;
#ifdef _DEBUG
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//�������ִ� �𿢽� ����
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//Create Swap Chain Setting

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1; // �� ���� ����
 

	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//����� ����
	sd.BufferDesc.RefreshRate.Numerator = 60;//�и�
	sd.BufferDesc.RefreshRate.Denominator = 1;//����

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // ����� ������
	sd.OutputWindow = g_hWnd; // ����� ������ �ڵ�
	sd.SampleDesc.Count = 1; // multisampling(����ǥ��ȭ) (MSAA ? Multi Sample Anti Aliased)
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		0,// �⺻ ���÷��� ����� ���
		D3D_DRIVER_TYPE_HARDWARE,// 3D �ϵ���� ����
		0,// ����Ʈ���� ���� ����
		createDeviceFlag,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,// SDK version
		&sd,// Swap chain description
		&g_pSwapChain,// ������ swapChain 
		&g_pd3dDevice,// ������ device
		&g_featureLevel,// ���� featureLevel
		&g_pImmediateContext// ������ ImmediateContext
	);

	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0// �ĸ� ���� �ε���. �������϶� �߿�. ������ 1���̹Ƿ� 
		, __uuidof(ID3D11Texture2D)// ���� ����
		, (LPVOID*)&pBackBuffer);// �޾ƿ� ����
	if (FAILED(hr))
	{
		return hr;
	}
	//�ؽ�ó�� View �������̽��� �����Ѵ�.RenderTargetView ������ ����

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer
		, NULL// �ڿ� ���� 
		, &g_pRenderTargetView);

	pBackBuffer->Release();
	if (FAILED(hr))
	{
		return hr;
	}
	//�� RenderTarget�� �ִ�ġ�� 8�̴�

	CreateDepthStencilTexture();

	g_pImmediateContext->OMSetRenderTargets(1// ���� ��� ����. ��� ���ҽ� 1 �ʰ�
		, &g_pRenderTargetView// ���� Ÿ��
		, g_pDepthStencilView);// ����/���ٽ� ����

	D3D11_VIEWPORT vp;
	vp.Width = 800;// ����Ʈ �ʺ�
	vp.Height = 600;// ����Ʈ ����
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;// �׸��� ���� ���� x
	vp.TopLeftY = 0;// �׸��� ���� ���� y
	g_pImmediateContext->RSSetViewports(1, &vp);//RS : rasterizer stage


	return hr;
}

void CalCulateMatrixForBox(float deltaTime)
{
	XMMATRIX mat = XMMatrixRotationY(deltaTime);
	mat *= XMMatrixRotationX(-deltaTime);
	g_World = mat;
	//g_World = XMMatrixIdentity();

	XMMATRIX wvp = g_World*g_View*g_Projection;
	ConstantBuffer cb;
	cb.wvp = XMMatrixTranspose(wvp);
	g_pImmediateContext->UpdateSubresource(g_PConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_PConstantBuffer);
}
void CalCulateMatrixForBox2(float deltaTime)
{
	XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX rotate = XMMatrixRotationZ(deltaTime);
	float moveValue = 5.0f;
	XMMATRIX position = XMMatrixTranslation(moveValue, 0.f, 0.f);
	g_World2 = scale *rotate*position;

	XMMATRIX rotate2 = XMMatrixRotationY(-deltaTime);
	g_World2 *= rotate2;
	//g_World = XMMatrixIdentity();

	XMMATRIX wvp = g_World2*g_View*g_Projection;
	ConstantBuffer cb;
	cb.wvp = XMMatrixTranspose(wvp);
	g_pImmediateContext->UpdateSubresource(g_PConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_PConstantBuffer);
}

void Render(float dt)
{
	// Just clear the backbuffer
	float clearColor[4] = { 0.f,0.f,0.f,1.0f };

	// Clear
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ������ ������
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

	CalCulateMatrixForBox2(dt);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
	CalCulateMatrixForBox(dt);
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	// Render (����۸� ����Ʈ���۷� �׸���.)
	g_pSwapChain->Present(0, 0);
}
void CleanupDevice()
{
	if (g_pDepthStencilView)g_pDepthStencilView->Release();
	if (g_PConstantBuffer)g_PConstantBuffer->Release();
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

//TODO: hr�� ���� �ڵ带 ������ �Ѵ�
void CreateShader()
{

	ID3DBlob *pErrorBlob = NULL;
	ID3DBlob *pVSBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(
		L"MyShader.fx",0, 0,// shader ���� ����
		"VS", "vs_5_0",// ������ ����
		0, 0, 0,// ���̴� �ɼ�
		&pVSBlob, &pErrorBlob, 0);// return value

	//���ؽ� ���̴� ����
	hr = g_pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),// �����ϵ� ���̴�
		pVSBlob->GetBufferSize(),// �����ϵ� ���̴� ũ��
		0,// ���� ���̴� ��ũ
		&g_pVertexShader);// ������ ���ؽ� ���̴� 

	//�Է� �����⿡�� ����� �Է� ���̾ƿ� ��ü ����

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(layout,// ���� ����
		numElements,// ���� ����
		pVSBlob->GetBufferPointer(),// �����ϵ� ���̴�
		pVSBlob->GetBufferSize(),// �����ϵ� ���̴� ũ��
		&g_pVertexLayout);// ������ Input layout

	pVSBlob->Release();

	ID3DBlob *pPSBlob = NULL;
	D3DX11CompileFromFile(L"MyShader.fx", 0, 0,// Shader ���� ����
		"PS", "ps_5_0",// ������ ����
		0, 0, 0,// ���̴� �ɼ�
		&pPSBlob, &pErrorBlob, 0);	// ����

	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPixelShader);

	pPSBlob->Release();
}

void CreateVertexBuffer()
{
	//������ ���� �����̳ʵ��� ���� 3d�� ������ �Է� �޴´�.
	MyVertex vertice[] =
	{
		{ XMFLOAT3(-1.0f,1.0f,-1.f),XMFLOAT4(0.f,0.f,1.f,1.f) },
		{ XMFLOAT3(1.f,1.0f,-1.f),XMFLOAT4(0.f,1.f,0.f,1.f) },
		{ XMFLOAT3(1.f,1.0f,1.f),XMFLOAT4(0.f,1.f,1.f,1.f) },
		{ XMFLOAT3(-1.f,1.f,1.f),XMFLOAT4(1.f,0.f,0.f,1.f) },
		{ XMFLOAT3(-1.f,-1.f,-1.f),XMFLOAT4(1.f,0.f,1.f,1.f) },
		{ XMFLOAT3(1.f,-1.f,-1.f),XMFLOAT4(1.f,1.f,0.f,1.f) },
		{ XMFLOAT3(1.f,-1.f,1.f),XMFLOAT4(1.f,1.f,1.f,1.f) },
		{ XMFLOAT3(-1.f,-1.f,1.f),XMFLOAT4(0.f,0.f,0.f,1.f) }
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertice);// ���� ũ��
	bd.Usage = D3D11_USAGE_DEFAULT;// ���� ��� ���
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;// ���������ο� ����Ǵ� ���� ����
	bd.CPUAccessFlags = 0;// CPU ���� flag. �Ϲ������� GPU�� ����ϱ� ������ 0�� ����.

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertice;// �ʱ�ȭ�ϱ� ���� ���� �迭 ������
	g_pd3dDevice->CreateBuffer(&bd,// ������ ������ ������ ���� ����ü
		&initData, // ���� �ʱ�ȭ�� �ʿ��� ������
		&g_pVertexBuffer);// ������ ����

}

void CreateIndexBuffer()
{
	UINT indices[] =
	{
		3,1,0,
		2,1,3,
		0,5,4,
		1,5,0,
		3,4,7,
		0,4,3,
		1,6,5,
		2,6,1,
		2,7,6,
		3,7,2,
		6,4,5,
		7,4,6,
	};

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(iinitData));
	iinitData.pSysMem = indices;
	g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer);
}

void CreateConstantBuffer()
{
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	//hr�� �޾Ƽ� Ȯ���� ����� �Ѵ�.
	g_pd3dDevice->CreateBuffer(&cbd, NULL, &g_PConstantBuffer);
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
	CreateVertexBuffer();// �ʱ�ȭ �۾��� ���ϰ� ũ�� ������ �ѹ��� ó��
	CreateIndexBuffer();

	CreateConstantBuffer();
	InitMatrix();

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
			static float deltaTime = 0;
			deltaTime += 0.003f;
			Render(deltaTime);
		}

	}
	CleanupDevice();
	return (int)msg.wParam;
}

void InitMatrix()
{
	//World Matrix
	g_World = XMMatrixIdentity();

	//View Matrix
	XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -8.0f, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	g_View = XMMatrixLookAtLH(pos, target, up);

	//Projection ���
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2,
		800.f / (FLOAT)600.f,
		0.3f, 1000.f);
}

void CreateDepthStencilTexture()
{
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = 800;
	descDepth.Height = 600;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDsv;
	ZeroMemory(&descDsv, sizeof(descDsv));
	descDsv.Format = descDepth.Format;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDsv.Texture2D.MipSlice = 0;
	descDsv.Flags = 0;
	g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDsv, &g_pDepthStencilView);
	g_pDepthStencil->Release();

}

// �޽��� ó�� �Լ�
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