#pragma once
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <DirectXMath.h>
#include <dxerr.h>
#include <d3dx11effect.h>
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11d.lib")
#pragma comment(lib,"Effects11d.lib")
namespace Mymap
{
	using namespace DirectX;

	class DirectXRenderer
	{
	public:
		DirectXRenderer();
		~DirectXRenderer();

		HRESULT InitDevice();

	private:

		//그릴 윈도우의 핸들.
		HWND m_hWnd = nullptr;
		
		//다이렉트 엑스의 그래픽 카드를 제어하는 인터페이스 기능 지원 점검이나 자원할당에 사용
		ID3D11Device* m_pD3DDevice = nullptr;
		
		//cpu에서 프로세스를 실행하기위한 여러가지들을 문맥이라고 했듯이 어떤대상을 어떤 파이프 라인을 
		//이용해서 그릴지 같은 gpu가 수행할 렌더링명령을 지시한다.  
		ID3D11DeviceContext* m_pImmediateContext = nullptr;
		
		//더블 버퍼링용 스왑체인
		IDXGISwapChain* m_swapChain = nullptr;
		
		//dx에서는 자원에 접근할때 직접 접근 하는 것이 아니라 자원에 대한 뷰를 이용해서 자원에 
		//접근한다.
		ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	
		//사용 된 dx버전 
		D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11InputLayout* m_pVertexLayout = nullptr;

		ID3D11Buffer* m_pVertexBuffer = nullptr;//ID3D11Buffer-> Vertex Buffer, Index Buffer.. 모든 버퍼 공통 인터페이스
		
		ID3D11Buffer* m_pIndexBuffer = nullptr;

		ID3D11Texture2D* m_pDepthStencil = nullptr;
		
		ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
		
		ID3D11ShaderResourceView* m_pTextureRV = nullptr;
		
		ID3D11SamplerState* m_pSamplerLinear = nullptr;

		ID3DX11Effect* g_FX = nullptr;
		
		ID3DX11EffectTechnique* g_Tech = nullptr;
		
		D3DX11_TECHNIQUE_DESC g_TechDesc;
		
		D3DX11_PASS_DESC g_PassDesc;
		
		ID3DX11EffectMatrixVariable*			g_FXWVP = nullptr;
		
		ID3DX11EffectMatrixVariable*			g_FXW = nullptr;
		
		ID3DX11EffectVectorVariable*			g_FXLightDir = nullptr;
		
		ID3DX11EffectVectorVariable*			g_FXLightColor = nullptr;
		
		ID3DX11EffectSamplerVariable*			g_SamLinear = nullptr;
	
		ID3DX11EffectShaderResourceVariable*	g_TexDiffuse = nullptr;

		XMMATRIX g_World;

		XMMATRIX g_World2;

		XMMATRIX g_View;

		XMMATRIX g_Projection;

		//Function
	private:
		HRESULT CreateDepthStencilTexture();

	};
	 

}
