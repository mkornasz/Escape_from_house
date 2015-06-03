# pragma once
#include <Windows.h>
#include "NuiApi.h"

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
		void SetSysMemSkeletonBuffer(BYTE* buffer);

	private:
		static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);

		HANDLE      m_hThNuiProcess;
		HANDLE      m_hEvNuiProcessStop;

		HANDLE      m_hNextSkeletonEvent;
		BYTE*		m_skeletonBuffer;
	};
}