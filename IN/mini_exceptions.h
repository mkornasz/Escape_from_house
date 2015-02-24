#pragma once

#include <Windows.h>
#include <string>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define STRINGIFY(x) #x
#define TOWSTRING(x) WIDEN(STRINGIFY(x))
#define __AT__ __WFILE__ L":" TOWSTRING(__LINE__)

namespace mini
{
	class Exception
	{
	public:
		Exception(const wchar_t* location) { m_location = location; }
		virtual std::wstring getMessage() const = 0;
		virtual int getExitCode() const = 0;
		const wchar_t* getErrorLocation() const { return m_location; }
	private:
		const wchar_t* m_location;
	};

	class WinAPIException : public mini::Exception
	{
	public:
		WinAPIException(const wchar_t* location, DWORD errorCode = GetLastError());
		virtual int getExitCode() const { return getErrorCode(); }
		inline DWORD getErrorCode() const { return m_code; }
		virtual std::wstring getMessage() const;

	private:
		DWORD m_code;
	};

	class Dx11Exception : public mini::Exception
	{
	public:
		Dx11Exception(const wchar_t* location, HRESULT result);
		virtual int getExitCode() const { return getResultCode(); }
		inline HRESULT getResultCode() const { return m_result; }
		virtual std::wstring getMessage() const;

	private:
		HRESULT m_result;
	};
}

#define THROW_WINAPI throw mini::WinAPIException(__AT__)
#define THROW_DX11(hr) throw mini::Dx11Exception(__AT__, hr)