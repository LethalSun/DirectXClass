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

		//�׸� �������� �ڵ�.
		HWND m_hWnd = nullptr;
		
		//���̷�Ʈ ������ �׷��� ī�带 �����ϴ� �������̽� ��� ���� �����̳� �ڿ��Ҵ翡 ���
		ID3D11Device* m_pD3DDevice = nullptr;
		
		//cpu���� ���μ����� �����ϱ����� ������������ �����̶�� �ߵ��� ������ � ������ ������ 
		//�̿��ؼ� �׸��� ���� gpu�� ������ ����������� �����Ѵ�.  
		ID3D11DeviceContext* m_pImmediateContext = nullptr;
		
		//���� ���۸��� ����ü��
		IDXGISwapChain* m_swapChain = nullptr;
		
		//dx������ �ڿ��� �����Ҷ� ���� ���� �ϴ� ���� �ƴ϶� �ڿ��� ���� �並 �̿��ؼ� �ڿ��� 
		//�����Ѵ�.
		ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	
		//��� �� dx���� 
		D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11InputLayout* m_pVertexLayout = nullptr;

		ID3D11Buffer* m_pVertexBuffer = nullptr;//ID3D11Buffer-> Vertex Buffer, Index Buffer.. ��� ���� ���� �������̽�
		
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
