#pragma once

#include "mini_windowApplication.h"
#include "mini_dxDevice.h"

#pragma warning( push )
#pragma warning( disable : 4005)
#include <d3d11.h>
#include <D3DX11.h>
#pragma warning( pop ) 

namespace mini
{
	class DxApplication : public mini::WindowApplication
	{
	public:
		DxApplication(HINSTANCE hInstance,
			int wndWidth = mini::Window::m_defaultWindowWidth,
			int wndHeight = mini::Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"DirectX Window")
			: mini::WindowApplication(hInstance, wndWidth, wndHeight, wndTitle) { }

	protected:
		virtual bool Initialize();
		virtual int MainLoop();
		virtual void Shutdown();

		virtual void Update(float dt) { }
		virtual void Render();

		mini::DxDevice m_device;
		mini::dx_ptr<ID3D11DeviceContext> m_context;
		mini::dx_ptr<IDXGISwapChain> m_swapChain;
		mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
		mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;


	private:
		void CreateDevice(int wndWidth, int wndHeight);
		void CreateBackBuffer(int wndWidth, int wndHeight);

		mini::dx_ptr<ID3D11Texture2D> m_backTexture, m_depthTexture;
	};
}