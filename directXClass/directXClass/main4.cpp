#define main4

#ifndef main4
//디버그이면
#pragma comment(lib,"Effects11d.lib")
//아니면 디버그아닌 버전
#include <d3dx11effect.h>


#include <windows.h>
#include <time.h>
//헤더와 기본 전역 변수
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <dxerr.h>
//DX12는 DirectXMath.h 를 사용해야 한다.

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
ID3D11Buffer* g_pVertexBuffer = NULL;// Vertex Buffer, Index Buffer.. 모든 버퍼 공통 인터페이스
using namespace DirectX;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11Buffer* g_pIndexBuffer;

//main3 추가
XMMATRIX g_World;
XMMATRIX g_World2;
XMMATRIX g_View;
XMMATRIX g_Projection;
ID3D11Buffer* g_PConstantBuffer;
ID3D11Texture2D* g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;

//main4 추가
ID3D11RasterizerState* g_pSolidRS;
ID3D11RasterizerState* g_pWireRS;
ID3D11ShaderResourceView* g_pTextureRV = NULL;
ID3D11SamplerState* g_pSamplerLinear = NULL;



struct MyVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;

	XMFLOAT3 normal;
	XMFLOAT2 tex;
};

struct ConstantBuffer
{
	XMMATRIX wvp;
	XMMATRIX world;

	XMFLOAT4 lightDir;
	XMFLOAT4 lightColor;
};

XMFLOAT4 lightDirection =
{ 
	XMFLOAT4(1.0f,1.0f,1.0f,1.0f), 
};

XMFLOAT4 lightColor =
{
	XMFLOAT4(1.0f,1.0f,0.0f,1.0f),
};
/*
struct MyVertex
{
XMFLOAT3 pos;

};
*/
void InitMatrix();
void CreateDepthStencilTexture();
void CreateRenderState();
HRESULT LoadTexture();

HRESULT InitDevice()
{

	//Create Device Setting
	HRESULT hr = S_OK;

	//Flag 설정
	UINT createDeviceFlag = 0;
#ifdef _DEBUG
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//지원해주는 디엑스 버전
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
	sd.BufferCount = 1; // 백 버퍼 갯수


	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//백버퍼 포맷
	sd.BufferDesc.RefreshRate.Numerator = 60;//분모
	sd.BufferDesc.RefreshRate.Denominator = 1;//분자

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 백버퍼 렌더링
	sd.OutputWindow = g_hWnd; // 출력할 윈도우 핸들
	sd.SampleDesc.Count = 1; // multisampling(다중표본화) (MSAA – Multi Sample Anti Aliased)
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		0,// 기본 디스플레이 어댑터 사용
		D3D_DRIVER_TYPE_HARDWARE,// 3D 하드웨어 가속
		0,// 소프트웨어 구동 안함
		createDeviceFlag,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,// SDK version
		&sd,// Swap chain description
		&g_pSwapChain,// 생성된 swapChain 
		&g_pd3dDevice,// 생성된 device
		&g_featureLevel,// 사용된 featureLevel
		&g_pImmediateContext// 생성된 ImmediateContext
	);

	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0// 후면 버퍼 인덱스. 여러개일때 중요. 지금은 1개이므로 
		, __uuidof(ID3D11Texture2D)// 버퍼 형식
		, (LPVOID*)&pBackBuffer);// 받아온 버퍼
	if (FAILED(hr))
	{
		return hr;
	}
	//텍스처는 View 인터페이스로 접근한다.RenderTargetView 생성의 이유

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer
		, NULL// 자원 형식 
		, &g_pRenderTargetView);

	pBackBuffer->Release();
	if (FAILED(hr))
	{
		return hr;
	}
	//※ RenderTarget의 최대치는 8이다

	CreateDepthStencilTexture();

	g_pImmediateContext->OMSetRenderTargets(1// 렌더 대상 개수. 장면 분할시 1 초과
		, &g_pRenderTargetView// 렌더 타겟
		, g_pDepthStencilView);// 깊이/스텐실 버퍼

	D3D11_VIEWPORT vp;
	vp.Width = 800;// 뷰포트 너비
	vp.Height = 600;// 뷰포트 높이
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;// 그리기 시작 원점 x
	vp.TopLeftY = 0;// 그리기 시작 원점 y
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

	cb.world = XMMatrixTranspose(g_World);
	cb.lightDir = lightDirection;
	cb.lightColor = lightColor;

	g_pImmediateContext->UpdateSubresource(g_PConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_PConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_PConstantBuffer);
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

	cb.world = XMMatrixTranspose(g_World2);
	cb.lightDir = lightDirection;
	cb.lightColor = lightColor;

	g_pImmediateContext->UpdateSubresource(g_PConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_PConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_PConstantBuffer);
}

void Render(float dt)
{
	// Just clear the backbuffer
	float clearColor[4] = { 0.4f,1.f,0.f,1.0f };

	// Clear
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 폴리곤 랜더링
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->RSSetState(g_pSolidRS);

	CalCulateMatrixForBox2(dt);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
	//g_pImmediateContext->RSSetState(g_pWireRS);
	CalCulateMatrixForBox(dt);
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	// Render (백버퍼를 프론트버퍼로 그린다.)
	g_pSwapChain->Present(0, 0);
}
void CleanupDevice()
{

	if (g_pWireRS)g_pWireRS->Release();
	if (g_pSamplerLinear)g_pSamplerLinear->Release();
	if (g_pTextureRV)g_pTextureRV->Release();
	if (g_pSolidRS)g_pSolidRS->Release();

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

	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
	}
	if (g_pImmediateContext)
	{
		g_pImmediateContext->ClearState();
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
		L"MyShader.fx", 0, 0,// shader 파일 설정
		"VS", "vs_5_0",// 컴파일 설정
		0, 0, 0,// 쉐이더 옵션
		&pVSBlob, &pErrorBlob, 0);// return value

								  //버텍스 쉐이더 생성
	hr = g_pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),// 컴파일된 쉐이더
		pVSBlob->GetBufferSize(),// 컴파일된 쉐이더 크기
		0,// 동적 쉐이더 링크
		&g_pVertexShader);// 생성된 버텍스 쉐이더 

						  //입력 조립기에서 사용할 입력 레이아웃 개체 생성

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(layout,// 성분 정의
		numElements,// 성분 개수
		pVSBlob->GetBufferPointer(),// 컴파일된 쉐이더
		pVSBlob->GetBufferSize(),// 컴파일된 쉐이더 크기
		&g_pVertexLayout);// 생성된 Input layout

	pVSBlob->Release();

	ID3DBlob *pPSBlob = NULL;
	D3DX11CompileFromFile(L"MyShader.fx", 0, 0,// Shader 파일 설정
		"PS", "ps_5_0",// 컴파일 설정
		0, 0, 0,// 쉐이더 옵션
		&pPSBlob, &pErrorBlob, 0);	// 리턴

	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPixelShader);

	pPSBlob->Release();
}

void CreateVertexBuffer()
{
	//정점들 보통 디자이너들이 만든 3d모델 정보를 입력 받는다.
	MyVertex vertice[] =
	{
		{ XMFLOAT3(-1.0f,1.0f,-1.f),XMFLOAT4(0.f,0.f,1.f,1.f),XMFLOAT3(-0.33f,0.33f,-0.33f)	,XMFLOAT2(1.0f,1.0f)},
		{ XMFLOAT3(1.f,1.0f,-1.f),XMFLOAT4(0.f,1.f,0.f,1.f)	,XMFLOAT3(0.33f,0.33f,-0.33f)	,XMFLOAT2(0.0f,1.0f)},
		{ XMFLOAT3(1.f,1.0f,1.f),XMFLOAT4(0.f,1.f,1.f,1.f)	,XMFLOAT3(0.33f,0.33f,0.33f)	,XMFLOAT2(0.0f,0.0f)},
		{ XMFLOAT3(-1.f,1.f,1.f),XMFLOAT4(1.f,0.f,0.f,1.f)	,XMFLOAT3(-0.33f,0.33f,0.33f)	,XMFLOAT2(1.0f,0.0f)},
		{ XMFLOAT3(-1.f,-1.f,-1.f),XMFLOAT4(1.f,0.f,1.f,1.f),XMFLOAT3(-0.33f,-0.33f,-0.33f)	,XMFLOAT2(0.0f,0.0f)},
		{ XMFLOAT3(1.f,-1.f,-1.f),XMFLOAT4(1.f,1.f,0.f,1.f)	,XMFLOAT3(0.33f,-0.33f,-0.33f)	,XMFLOAT2(1.0f,0.0f)},
		{ XMFLOAT3(1.f,-1.f,1.f),XMFLOAT4(1.f,1.f,1.f,1.f)	,XMFLOAT3(0.33f,-0.33f,0.33f)	,XMFLOAT2(1.0f,1.0f)},
		{ XMFLOAT3(-1.f,-1.f,1.f),XMFLOAT4(0.f,0.f,0.f,1.f)	,XMFLOAT3(-0.33f,-0.33f,0.33f)	,XMFLOAT2(0.0f,1.0f)}
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertice);// 버퍼 크기
	bd.Usage = D3D11_USAGE_DEFAULT;// 버퍼 사용 방식
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 파이프라인에 연결되는 버퍼 형태
	bd.CPUAccessFlags = 0;// CPU 접근 flag. 일반적으로 GPU를 사용하기 때문에 0을 쓴다.

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertice;// 초기화하기 위한 버퍼 배열 포인터
	g_pd3dDevice->CreateBuffer(&bd,// 생성할 버퍼의 정보를 담은 구조체
		&initData, // 버퍼 초기화시 필요한 데이터
		&g_pVertexBuffer);// 생성된 버퍼

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
	//hr을 받아서 확인을 해줘야 한다.
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
	CreateVertexBuffer();// 초기화 작업은 부하가 크기 때문에 한번만 처리
	CreateIndexBuffer();

	CreateConstantBuffer();

	CreateRenderState();
	LoadTexture();
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

	//Projection 행렬
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


void CreateRenderState()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;

	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &g_pSolidRS);

	D3D11_RASTERIZER_DESC rasterizerDesc1;
	ZeroMemory(&rasterizerDesc1, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc1.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc1.CullMode = D3D11_CULL_BACK;
	rasterizerDesc1.FrontCounterClockwise = true;


	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc1, &g_pWireRS);
}

HRESULT LoadTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		g_pd3dDevice,
		L"images.jpg",
		NULL,
		NULL,
		&g_pTextureRV,
		NULL
	);
	//hr 확인
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	//hr 확인
	return hr;
}
#endif // !main2
