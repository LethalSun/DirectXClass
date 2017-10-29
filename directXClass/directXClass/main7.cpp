#define Main71

#ifndef Main7
#define _XM_NO_INTRINSICS_
#include <windows.h>
#include <time.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <DirectXMath.h>
#include <WindowsX.h>
#include <d3dx11effect.h>
//#include <xnamath.h>
#include <fstream>
#include <vector>
#include "Camera.h"

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11d.lib")
#pragma comment(lib,"Effects11d.lib")

#define szWindowClass	TEXT("First")
#define szTitle			TEXT("First App")

using namespace std;
using namespace DirectX;

XMFLOAT4 lightDirection =
{
	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
};

XMFLOAT4 lightColor =
{
	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
};

static const XMVECTOR g_UnitVectorEpsilon =
{
	1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f
};

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

HWND g_hWnd = NULL;
IDXGISwapChain *g_pSwapChain = NULL;
ID3D11Device *g_pd3dDevice = NULL;
ID3D11DeviceContext *g_pImmediateContext = NULL;
ID3D11RenderTargetView *g_pRenderTargetView = NULL;
ID3D11VertexShader *g_pVertexShader = NULL;
ID3D11InputLayout *g_pVertexLayout = NULL;
ID3D11Buffer *g_pVertexBuffer = NULL;
ID3D11PixelShader *g_pPixelShader = NULL;
ID3D11Buffer *g_pIndexBuffer;
XMMATRIX g_World, g_World2;
//XMMATRIX g_View;
//XMMATRIX g_Projection;
ID3D11Buffer *g_pConstantBuffer;
ID3D11Texture2D *g_pDepthStencil = NULL;
ID3D11DepthStencilView *g_pDepthStencilView = NULL;
ID3D11RasterizerState *g_pSolidRS, *g_pWireRS;
ID3D11ShaderResourceView *g_pTextureRV = NULL;
ID3D11SamplerState *g_pSamplerLinear = NULL;

int vertexCount = 100;
int numVertices = 10000;
ID3D11Buffer *g_pHeightMapVertexBuffer;
int g_GridIndexSize = 0;
ID3D11Buffer *g_pHeightMapIndexBuffer;

ID3D11DepthStencilState *g_LessEqualDSS;
UINT *g_GridIndices;
XMMATRIX g_gridWorld;
MyVertex *g_gridVertex;
ID3D11ShaderResourceView *g_pHeightmapRV = NULL;
int g_GridPickedTraingle = -1;
ID3D11PixelShader *g_pPickedPixelShader = NULL;
//ID3D11SamplerState *g_pSamplerLinear = NULL;

Camera g_Camera;
float g_ClientWidth = 800.f;
float g_ClientHeight = 600.f;
float g_PickRange = 15.0f;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;

void InitMatrix()
{
	g_World = XMMatrixIdentity();

}
void CreateConstantBuffer()
{
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	g_pd3dDevice->CreateBuffer(&cbd, NULL, &g_pConstantBuffer);
}
void CalculateMatrixForBox(float deltaTime)
{
	XMMATRIX mat = XMMatrixRotationY(deltaTime);
	mat *= XMMatrixRotationX(-deltaTime);
	g_World = mat;

	XMMATRIX wvp = g_World * g_Camera.ViewProj();/*g_View * g_Projection*/;
	ConstantBuffer cb;

	cb.wvp = XMMatrixTranspose(wvp);
	cb.world = XMMatrixTranspose(g_World);
	cb.lightDir = lightDirection;
	cb.lightColor = lightColor;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}
void CalculateMatrixForBox2(float deltaTime)
{
	float scaleValue = 0.5f;//sinf(deltaTime) * 0.5f + 1;
	XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
	XMMATRIX rotate = XMMatrixRotationY(deltaTime);
	float moveValue = cosf(deltaTime) * 5.0f;
	//XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 0.0f);
	XMMATRIX position = XMMatrixTranslation(3.f, 0.0f, 0.0f);
	g_World2 = XMMatrixRotationZ(-deltaTime) * scale * position * rotate;//scale * rotate * position;

	XMMATRIX wvp = g_World2 * g_Camera.ViewProj();//g_View * g_Projection;
	ConstantBuffer cb;
	cb.wvp = XMMatrixTranspose(wvp);
	cb.world = XMMatrixTranspose(g_World);
	cb.lightDir = lightDirection;
	cb.lightColor = lightColor;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}
void CalcMatrixForHeightMap(float deltaTime)
{
	XMMATRIX mat = XMMatrixIdentity();//XMMatrixRotationY(deltaTime/2.f);
	g_gridWorld = mat;

	XMMATRIX wvp = g_gridWorld *g_Camera.ViewProj();// g_View * g_Projection;
	ConstantBuffer cb;
	cb.wvp = XMMatrixTranspose(wvp);
	cb.world = XMMatrixTranspose(g_World);
	cb.lightDir = lightDirection;
	cb.lightColor = lightColor;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, 0, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

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


	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	descDSV.Flags = 0;
	g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);


	D3D11_DEPTH_STENCIL_DESC lessEqualDesc;
	lessEqualDesc.DepthEnable = true;
	lessEqualDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	lessEqualDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	lessEqualDesc.StencilEnable = false;

	g_pd3dDevice->CreateDepthStencilState(&lessEqualDesc, &g_LessEqualDSS);
}

void CreateHeightMapVB()
{
	g_gridVertex = new MyVertex[numVertices];
	for (int z = 0; z < vertexCount; ++z)
	{
		for (int x = 0; x < vertexCount; ++x)
		{
			int idx = x + (z*vertexCount);
			g_gridVertex[idx].pos = XMFLOAT3(x, 1.0f, z);//XMFLOAT3(x, sinf(idx) * 3.f, z);
			g_gridVertex[idx].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			g_gridVertex[idx].tex = XMFLOAT2(x / (float)(vertexCount - 1), z / (float)(vertexCount - 1));
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(MyVertex) * numVertices;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = g_gridVertex;
	g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pHeightMapVertexBuffer);
}
void CreateHeightMapIB()
{
	int trigngleCount = (vertexCount - 1) * (vertexCount - 1) * 2;
	g_GridIndexSize = trigngleCount * 3;
	g_GridIndices = new UINT[g_GridIndexSize];

	int baseIndex = 0;
	int _numVertsPerRow = vertexCount;
	for (int z = 0; z < _numVertsPerRow - 1; ++z)
	{
		for (int x = 0; x < _numVertsPerRow - 1; ++x)
		{
			g_GridIndices[baseIndex] = z * _numVertsPerRow + x;
			g_GridIndices[baseIndex + 2] = z * _numVertsPerRow + x + 1;
			g_GridIndices[baseIndex + 1] = (z + 1) * _numVertsPerRow + x;

			g_GridIndices[baseIndex + 3] = (z + 1) * _numVertsPerRow + x;
			g_GridIndices[baseIndex + 5] = z * _numVertsPerRow + x + 1;
			g_GridIndices[baseIndex + 4] = (z + 1) * _numVertsPerRow + x + 1;

			baseIndex += 6;
		}
	}
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(UINT) * g_GridIndexSize;
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = g_GridIndices;
	g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pHeightMapIndexBuffer);
}
void UpdateGridVertices()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pImmediateContext->Map(g_pHeightMapVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, g_gridVertex, sizeof(MyVertex) * numVertices);
	g_pImmediateContext->Unmap(g_pHeightMapVertexBuffer, 0);
}

HRESULT LoadTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		g_pd3dDevice,
		L"image.jpg",
		NULL,
		NULL,
		&g_pTextureRV,
		NULL
	);

	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	return hr;
}
HRESULT LoadHeightmapTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		g_pd3dDevice,
		L"heightMap.jpg",
		NULL,
		NULL,
		&g_pHeightmapRV,
		NULL
	);

	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	return hr;
}
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//스왑체인 만들기위한 설정정보
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1; //백버퍼 갯수

	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//백버퍼 포맷
	sd.BufferDesc.RefreshRate.Numerator = 60;//분자
	sd.BufferDesc.RefreshRate.Denominator = 1;//분모

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //백버퍼 랜더링용.
	sd.OutputWindow = g_hWnd;//출력할 윈도우 핸들
	sd.SampleDesc.Count = 1;//다중 표본화->안티 앨리어싱,퀄리티는 그래픽카드마다 다름 함수를 이용해 확인 해야 함
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		0,//기본 디스플레이 어뎁터 사용
		D3D_DRIVER_TYPE_HARDWARE,//3d하드웨어 가속
		0,//소프트 웨어 구동 안함
		createDeviceFlags,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,
		&sd,//스왑체인 디스크립션
		&g_pSwapChain,//생성된 스왑체인
		&g_pd3dDevice,//생성된 디바이스
		&g_featureLevel,//사용된 featureLevels
		&g_pImmediateContext);//생성된 ImmediateContext 디바이스 컨텍스트 
							  //다이렉트 엑스에서 렌더대상 설정및 자원을 연결 
							  //gpu에 렌더링 명령 수행와 같은 역할.

	if (FAILED(hr))
		return hr;

	ID3D11Texture3D *pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(
		0,//후면 버퍼 인덱스 지금은 1개 이므로 0
		__uuidof(ID3D11Texture2D),//버퍼 형식
		(LPVOID *)&pBackBuffer);//받아온 버퍼

	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer,
		NULL,//자원 형식
		&g_pRenderTargetView);//텍스쳐는 뷰로 접근해야 한다.
	pBackBuffer->Release();//사용한 후에는 반드시 릴리즈

	if (FAILED(hr))
		return hr;

	CreateDepthStencilTexture();//뎁스 스텐실 텍스쳐를 만들고, 그것에 대한 뷰를 만든다.
	g_pImmediateContext->OMSetRenderTargets(1,
		&g_pRenderTargetView,//렌더 타겟
		g_pDepthStencilView);//뎁스 스텐실버퍼

	D3D11_VIEWPORT vp;//뷰포트
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);//뷰포트 설정

	return hr;
}
void CleanupDevice()
{
	delete g_gridVertex;
	delete g_GridIndices;

	if (g_LessEqualDSS) g_LessEqualDSS->Release();
	if (g_pPickedPixelShader) g_pPickedPixelShader->Release();
	if (g_pHeightMapIndexBuffer) g_pHeightMapIndexBuffer->Release();
	if (g_pHeightMapVertexBuffer) g_pHeightMapVertexBuffer->Release();

	if (g_pTextureRV) g_pTextureRV->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pWireRS) g_pWireRS->Release();
	if (g_pSolidRS) g_pSolidRS->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pConstantBuffer) g_pConstantBuffer->Release();

	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}
void Render(float deltaTime)
{
	g_Camera.UpdateViewMatrix();
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;

	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pHeightMapVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pHeightMapIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pHeightmapRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

	CalcMatrixForHeightMap(deltaTime);
	g_pImmediateContext->DrawIndexed(g_GridIndexSize, 0, 0);

	if (g_GridPickedTraingle != -1) {
		g_pImmediateContext->OMSetDepthStencilState(g_LessEqualDSS, 0);
		g_pImmediateContext->PSSetShader(g_pPickedPixelShader, NULL, 0);
		g_pImmediateContext->DrawIndexed(3, 3 * g_GridPickedTraingle, 0);

		g_pImmediateContext->OMSetDepthStencilState(0, 0);
	}

	g_pSwapChain->Present(0, 0);
}

//쉐이더를 만든다.
void CreateShader()
{
	ID3DBlob *pErrorBlob = NULL;
	ID3DBlob *pVSBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(L"MyShader.fx", 0, 0,
		"VS", "vs_5_0",
		0, 0, 0,
		&pVSBlob, &pErrorBlob, 0);

	hr = g_pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		0, &g_pVertexShader);

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT      , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_pVertexLayout);
	pVSBlob->Release();

	ID3DBlob *pPSBlob = NULL;
	hr = D3DX11CompileFromFile(L"MyShader.fx", 0, 0,
		"PS", "ps_5_0",
		0, 0, 0,
		&pPSBlob, &pErrorBlob, 0);
	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPixelShader);
	pPSBlob->Release();

	hr = D3DX11CompileFromFile(L"pick.fx", 0, 0,
		"PS", "ps_5_0",
		0, 0, 0,
		&pPSBlob, &pErrorBlob, 0);
	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPickedPixelShader);
	pPSBlob->Release();
}
void CreateVertexBuffer()
{
	MyVertex vertices[] = {
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-0.33f,  0.33f, -0.33f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(0.33f,  0.33f, -0.33f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.33f,  0.33f,  0.33f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-0.33f,  0.33f,  0.33f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-0.33f, -0.33f, -0.33f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(0.33f, -0.33f, -0.33f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.33f, -0.33f,  0.33f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-0.33f, -0.33f,  0.33f), XMFLOAT2(0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	g_pd3dDevice->CreateBuffer(&bd,
		&initData,
		&g_pVertexBuffer);
}
void CreateIndexBuffer()
{
	UINT indices[] = {
		3, 1, 0,
		2, 1, 3,
		0, 5, 4,
		1, 5, 0,
		3, 4, 7,
		0, 4, 3,
		1, 6, 5,
		2, 6, 1,
		2, 7, 6,
		3, 7, 2,
		6, 4, 5,
		7, 4, 6
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(indices);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = indices;
	g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
}
void CreateRenderState()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;

	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &g_pSolidRS);

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &g_pWireRS);
}

static inline BOOL XMVector3IsUnit(FXMVECTOR V)
{
	XMVECTOR Difference = XMVector3Length(V) - XMVectorSplatOne();

	return XMVector4Less(XMVectorAbs(Difference), g_UnitVectorEpsilon);
}
BOOL IntersectRayTriangle(FXMVECTOR Origin, FXMVECTOR Direction, FXMVECTOR V0, CXMVECTOR V1, CXMVECTOR V2, FLOAT* pDist)
{
	//XMASSERT(pDist);
	//XMASSERT(XMVector3IsUnit(Direction));

	static const XMVECTOR Epsilon =
	{
		1e-20f, 1e-20f, 1e-20f, 1e-20f
	};

	XMVECTOR Zero = XMVectorZero();

	XMVECTOR e1 = V1 - V0;
	XMVECTOR e2 = V2 - V0;

	XMVECTOR p = XMVector3Cross(Direction, e2);

	XMVECTOR det = XMVector3Dot(e1, p);

	XMVECTOR u, v, t;

	if (XMVector3GreaterOrEqual(det, Epsilon))
	{
		XMVECTOR s = Origin - V0;

		u = XMVector3Dot(s, p);

		XMVECTOR NoIntersection = XMVectorLess(u, Zero);
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u, det));

		XMVECTOR q = XMVector3Cross(s, e1);

		v = XMVector3Dot(Direction, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(v, Zero));
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u + v, det));

		t = XMVector3Dot(e2, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(t, Zero));

		if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
			return FALSE;
	}
	else if (XMVector3LessOrEqual(det, -Epsilon))
	{
		XMVECTOR s = Origin - V0;

		u = XMVector3Dot(s, p);

		XMVECTOR NoIntersection = XMVectorGreater(u, Zero);
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u, det));

		XMVECTOR q = XMVector3Cross(s, e1);

		v = XMVector3Dot(Direction, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(v, Zero));
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u + v, det));

		t = XMVector3Dot(e2, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(t, Zero));

		if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	XMVECTOR inv_det = XMVectorReciprocal(det);

	t *= inv_det;

	XMStoreFloat(pDist, t);

	return TRUE;
}
void Pick(int sx, int sy)
{
	XMMATRIX P = g_Camera.Proj();

	float vx = (+2.0f*sx / g_ClientWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f*sy / g_ClientHeight + 1.0f) / P(1, 1);

	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	XMMATRIX V = g_Camera.View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = g_gridWorld;
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);
	rayDir = XMVector3Normalize(rayDir);

	g_GridPickedTraingle = -1;
	float tmin = 0.0f;

	tmin = 10000.f;
	for (UINT i = 0; i < g_GridIndexSize / 3; ++i)
	{
		UINT i0 = g_GridIndices[i * 3 + 0];
		UINT i1 = g_GridIndices[i * 3 + 1];
		UINT i2 = g_GridIndices[i * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3(&g_gridVertex[i0].pos);
		XMVECTOR v1 = XMLoadFloat3(&g_gridVertex[i1].pos);
		XMVECTOR v2 = XMLoadFloat3(&g_gridVertex[i2].pos);

		float t = 0.0f;
		if (IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &t))
		{
			if (t < tmin)
			{
				tmin = t;
				g_GridPickedTraingle = i;
			}
		}
	}
}

float GetSinHeight(float stndHeight, float distance)
{
	auto normDist = distance / g_PickRange;

	if (normDist > 1.0f) return FLT_MAX;

	float sinInput = (float)1.57079632679489661923 + (float)1.57079632679489661923 * normDist;

	return stndHeight * sinf(sinInput);
};

float GetDistanceBetweenTwoVerticesInXZPlane(float x1, float z1, float x2, float z2)
{
	auto dx = x1 - x2;
	auto dz = z1 - z2;

	auto dist = sqrtf(dx * dx + dz * dz);

	return dist;
};

//Pick round range
std::vector<int> GetVerticesInRange(int pickedTraingle, float range)
{
	std::vector<int>VertexIndicesInRange;

	if (pickedTraingle == -1)
	{
		return VertexIndicesInRange;
	}

	auto refVertexIndex = g_GridIndices[(g_GridPickedTraingle * 3)];
	auto refVertex = g_gridVertex[refVertexIndex];
	for (int i = 0; i < numVertices; ++i)
	{
		auto distance = GetDistanceBetweenTwoVerticesInXZPlane(
			refVertex.pos.x, 
			refVertex.pos.z, 
			g_gridVertex[i].pos.x, 
			g_gridVertex[i].pos.z);

		if (distance < range)
		{
			VertexIndicesInRange.push_back(i);
		}

	}

	return VertexIndicesInRange;
}


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
	InitDevice();//스왑체인,디바이스를 생성하고, 이미디어트 컨텍스트를 받아온다
				 //렌더 타겟뷰
				 //뷰포트와 뎁스 스텐실 버퍼를 만들고
	CreateShader();

	CreateHeightMapVB();
	CreateHeightMapIB();

	CreateConstantBuffer();
	CreateRenderState();
	LoadTexture();
	LoadHeightmapTexture();
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
			static float deltaTime = 0.f;
			float speed = 0.1f;
			deltaTime += 0.0005f;

			if (GetAsyncKeyState('W') & 0x8000)
				g_Camera.Walk(speed);
			if (GetAsyncKeyState('S') & 0x8000)
				g_Camera.Walk(-speed);
			if (GetAsyncKeyState('A') & 0x8000)
				g_Camera.Strafe(-speed);
			if (GetAsyncKeyState('D') & 0x8000)
				g_Camera.Strafe(speed);

			if (g_GridPickedTraingle != -1) {

				
				float factor = 0.05f;

				auto targetIndices = GetVerticesInRange(g_GridPickedTraingle,15.0f);
				for (const auto& index : targetIndices)
				{

					auto dist = GetDistanceBetweenTwoVerticesInXZPlane(
						g_gridVertex[g_GridIndices[(g_GridPickedTraingle * 3)]].pos.x,
						g_gridVertex[g_GridIndices[(g_GridPickedTraingle * 3)]].pos.z,
						g_gridVertex[index].pos.x,
						g_gridVertex[index].pos.z);

					auto newHeight = GetSinHeight(g_gridVertex[g_GridIndices[(g_GridPickedTraingle * 3)]].pos.y, dist);

					if (GetAsyncKeyState(VK_UP) & 0x8000) {
						g_gridVertex[index].pos.y += factor*newHeight;

						UpdateGridVertices();
					}

					if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
						g_gridVertex[index].pos.y -= factor*newHeight;

						UpdateGridVertices();
					}
				}

			}

			Render(deltaTime);
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

	static float mLastMouseX = 0.0f;
	static float mLastMouseY = 0.0f;

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
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	case WM_SIZE:
	{
		g_ClientWidth = LOWORD(lParam);
		g_ClientHeight = HIWORD(lParam);

		g_Camera.SetLens(XM_PIDIV2, g_ClientWidth / g_ClientHeight, 0.01f, 1000.0f);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);

		Pick(x, y);
	}
	break;
	case WM_MOUSEMOVE:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);

		if ((wParam & MK_LBUTTON) != 0) {
			float dx = XMConvertToRadians(0.2f * static_cast<float>(x - mLastMouseX));
			float dy = XMConvertToRadians(0.2f * static_cast<float>(y - mLastMouseY));

			g_Camera.Pitch(dy);
			g_Camera.RotateY(dx);
		}
		mLastMouseX = x;
		mLastMouseY = y;
	}
	break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif // !Main7
