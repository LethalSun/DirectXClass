#include "DirectXRenderer.h"

namespace Mymap
{
	DirectXRenderer::DirectXRenderer()
	{
	}

	DirectXRenderer::~DirectXRenderer()
	{
	}

	HRESULT DirectXRenderer::InitDevice()
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

		//����ü�� ��������� ��������
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1; //����� ����

		sd.BufferDesc.Width = 800;
		sd.BufferDesc.Height = 600;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//����� ����
		sd.BufferDesc.RefreshRate.Numerator = 60;//����
		sd.BufferDesc.RefreshRate.Denominator = 1;//�и�

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //����� ��������.
		sd.OutputWindow = m_hWnd;//����� ������ �ڵ�
		sd.SampleDesc.Count = 1;//���� ǥ��ȭ->��Ƽ �ٸ����,����Ƽ�� �׷���ī�帶�� �ٸ� �Լ��� �̿��� Ȯ�� �ؾ� ��
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = D3D11CreateDeviceAndSwapChain(
			0,//�⺻ ���÷��� ��� ���
			D3D_DRIVER_TYPE_HARDWARE,//3d�ϵ���� ����
			0,//����Ʈ ���� ���� ����
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,//����ü�� ��ũ����
			&m_swapChain,//������ ����ü��
			&m_pD3DDevice,//������ ����̽�
			&m_FeatureLevel,//���� featureLevels
			&m_pImmediateContext);//������ ImmediateContext ����̽� ���ؽ�Ʈ 
								  //���̷�Ʈ �������� ������� ������ �ڿ��� ���� 
								  //gpu�� ������ ��� ����� ���� ����.

		if (FAILED(hr))
			return hr;

		ID3D11Texture3D *pBackBuffer = NULL;
		hr = m_swapChain->GetBuffer(
			0,//�ĸ� ���� �ε��� ������ 1�� �̹Ƿ� 0
			__uuidof(ID3D11Texture2D),//���� ����
			(LPVOID *)&pBackBuffer);//�޾ƿ� ����

		if (FAILED(hr))
			return hr;

		hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer,
			NULL,//�ڿ� ����
			&m_pRenderTargetView);//�ؽ��Ĵ� ��� �����ؾ� �Ѵ�.
		pBackBuffer->Release();//����� �Ŀ��� �ݵ�� ������

		if (FAILED(hr))
			return hr;

		//CreateDepthStencilTexture();//���� ���ٽ� �ؽ��ĸ� �����, �װͿ� ���� �並 �����.
		m_pImmediateContext->OMSetRenderTargets(1,
			&m_pRenderTargetView,//���� Ÿ��
			m_pDepthStencilView);//���� ���ٽǹ���

		D3D11_VIEWPORT vp;//����Ʈ
		vp.Width = 800;
		vp.Height = 600;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		//g_pImmediateContext->RSSetViewports(1, &vp);//����Ʈ ����

		return hr;
	}
	HRESULT DirectXRenderer::CreateDepthStencilTexture()
	{
		return E_NOTIMPL;
	}
}


