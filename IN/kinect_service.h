# pragma once
#include <Windows.h>

namespace mini
{
	class KinectService
	{
	public:
		KinectService();
		~KinectService();

		bool Initialize();
		void Shutdown();

		void Nui_GotSkeletonAlert();

	private:
		static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);

		HANDLE      m_hThNuiProcess;
		HANDLE      m_hEvNuiProcessStop;

		HANDLE      m_hNextSkeletonEvent;
		BYTE*		m_pSysMemSkeletonBuffer;
	};
}