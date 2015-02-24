#pragma once

#include <string>
#include <Windows.h>

namespace mini
{
	/*************************************************************************
	 Wraps a standard WINAPI window message
	 *************************************************************************/
	struct WindowMessage
	{
		UINT message;
		WPARAM wParam;
		LPARAM lParam;
		LRESULT result;
	};

	/*************************************************************************
	 Interface that should be implemented by a class that wants to recieve
	 notifications about messages send to a window.
	 *************************************************************************/
	class IWindowMessageHandler
	{
	public:
		/*********************************************************************
		 Called whenever window recieves a message.
		 m contains message ID and its parameters.
		 If the message is processed method should return true and m.result
		 should be set to the result of message processing.
	     *********************************************************************/
		virtual bool ProcessMessage(mini::WindowMessage& m) = 0;
	};

	class Window
	{
	public:
		
		static const int m_defaultWindowWidth;
		static const int m_defaultWindowHeight;

		Window(HINSTANCE hInstance, int width, int height,
			   mini::IWindowMessageHandler* h = NULL);
		Window(HINSTANCE hInstance, int width, int height, const std::wstring& title,
			   mini::IWindowMessageHandler* h = NULL);
		virtual ~Window();

		virtual void Show(int cmdShow);
		SIZE getClientSize() const;
		RECT getClientRectangle() const;
		inline HWND getHandle() const { return m_hWnd; }

	protected:
		virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static bool IsWindowClassRegistered(HINSTANCE hInstance);
		static void RegisterWindowClass(HINSTANCE hInstance);
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static std::wstring m_windowClassName;

		Window(const mini::Window& other) { /* Do not use!*/ }

		void CreateWindowHandle(int width, int height, const std::wstring& title);

		HWND m_hWnd;
		HINSTANCE m_hInstance;
		mini::IWindowMessageHandler * m_messageHandler;
	};
}