#include "mini_dxApplication.h"
#include "mini_dxstructures.h"
#include "mini_exceptions.h"

using namespace mini;
using namespace mini::utils;

int DxApplication::MainLoop()
{
	MSG msg = { 0 };
	float t = 0.0f;
	LARGE_INTEGER timeStart = { 0 };
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			LARGE_INTEGER timeCurrent;
			QueryPerformanceCounter(&timeCurrent);
			if (timeStart.QuadPart == 0)
				timeStart = timeCurrent;
			t = (timeCurrent.QuadPart - timeStart.QuadPart)/static_cast<double>(freq.QuadPart);
			timeStart = timeCurrent;
			Update(t);
			Render();
			m_swapChain->Present(0, 0);
		}
	}
	return static_cast<int>(msg.wParam);
}

void DxApplication::CreateDevice(int wndWidth, int wndHeight)
{
	D3D_DRIVER_TYPE driverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE };
	unsigned int driverTypesCount = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	unsigned int featureLevelsCout = ARRAYSIZE(featureLevels);
	SwapChainDescription desc(m_window.getHandle(), wndWidth, wndHeight);
	unsigned int creationFlags = 0;
//#ifdef _DEBUG
//	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
	HRESULT result = S_OK;
	D3D_FEATURE_LEVEL fl;
	for (unsigned int driver = 0; driver < driverTypesCount; ++driver)
	{
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain* swapChain = nullptr;
		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, creationFlags, featureLevels,
			featureLevelsCout, D3D11_SDK_VERSION, &desc, &swapChain, &device, &fl, &context);
		m_device.Initialize(device);
		m_swapChain.reset(swapChain);
		m_context.reset(context);
		if (SUCCEEDED(result))
		{
			return;
		}
	}
	THROW_DX11(result);
}

void DxApplication::CreateBackBuffer(int wndWidth, int wndHeight)
{
	ID3D11Texture2D* bbt;
	HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&bbt);
	m_backTexture.reset(bbt);
	if (FAILED(result))
		THROW_DX11(result);
	m_backBuffer = m_device.CreateRenderTargetView(m_backTexture);
	auto desc = Texture2DDescription::DepthTextureDescription(wndWidth, wndHeight);
	m_depthTexture = m_device.CreateTexture(desc);
	m_depthBuffer= m_device.CreateDepthStencilView(m_depthTexture);
	ID3D11RenderTargetView* backBuffer = m_backBuffer.get();
	m_context->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
	ViewportDescription viewport(wndWidth, wndHeight);
	m_context->RSSetViewports(1, &viewport);
}

bool DxApplication::Initialize()
{
	if (!WindowApplication::Initialize())
		return false;
	SIZE windowSize = m_window.getClientSize();
	CreateDevice(windowSize.cx, windowSize.cy);
	CreateBackBuffer(windowSize.cx, windowSize.cy);
	return true;
}

void DxApplication::Shutdown()
{
	m_depthBuffer.reset();
	m_depthTexture.reset();
	m_backBuffer.reset();
	m_backTexture.reset();
	m_swapChain.reset();
	m_context.reset();
	m_device.Release();
	WindowApplication::Shutdown();
}

void DxApplication::Render()
{
	float clearColor[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}