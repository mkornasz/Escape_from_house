#pragma once

#include "mini_window.h"
#include <memory>

namespace mini
{
	class WindowApplication : public IWindowMessageHandler
	{
	public:
		WindowApplication(HINSTANCE hInstance,
			int wndWidht = mini::Window::m_defaultWindowWidth,
			int wndHeight = mini::Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"Default Window");
		virtual ~WindowApplication();

		int Run(int cmdShow = SW_SHOWNORMAL);
		inline HINSTANCE getHandle() const { return m_hInstance; }
		virtual bool ProcessMessage(mini::WindowMessage& msg) { return false; }

	protected:
		virtual bool Initialize() { return true; }
		virtual int MainLoop();
		virtual void Shutdown() { };

		Window m_window;

	private:
		HINSTANCE m_hInstance;
	};
}