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
		sd.OutputWindow = m_hWnd;//출력할 윈도우 핸들
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
			&m_swapChain,//생성된 스왑체인
			&m_pD3DDevice,//생성된 디바이스
			&m_FeatureLevel,//사용된 featureLevels
			&m_pImmediateContext);//생성된 ImmediateContext 디바이스 컨텍스트 
								  //다이렉트 엑스에서 렌더대상 설정및 자원을 연결 
								  //gpu에 렌더링 명령 수행와 같은 역할.

		if (FAILED(hr))
			return hr;

		ID3D11Texture3D *pBackBuffer = NULL;
		hr = m_swapChain->GetBuffer(
			0,//후면 버퍼 인덱스 지금은 1개 이므로 0
			__uuidof(ID3D11Texture2D),//버퍼 형식
			(LPVOID *)&pBackBuffer);//받아온 버퍼

		if (FAILED(hr))
			return hr;

		hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer,
			NULL,//자원 형식
			&m_pRenderTargetView);//텍스쳐는 뷰로 접근해야 한다.
		pBackBuffer->Release();//사용한 후에는 반드시 릴리즈

		if (FAILED(hr))
			return hr;

		//CreateDepthStencilTexture();//뎁스 스텐실 텍스쳐를 만들고, 그것에 대한 뷰를 만든다.
		m_pImmediateContext->OMSetRenderTargets(1,
			&m_pRenderTargetView,//렌더 타겟
			m_pDepthStencilView);//뎁스 스텐실버퍼

		D3D11_VIEWPORT vp;//뷰포트
		vp.Width = 800;
		vp.Height = 600;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		//g_pImmediateContext->RSSetViewports(1, &vp);//뷰포트 설정

		return hr;
	}
	HRESULT DirectXRenderer::CreateDepthStencilTexture()
	{
		return E_NOTIMPL;
	}
}


