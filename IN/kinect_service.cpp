#include "kinect_service.h"
#include "NuiApi.h"

using namespace mini;

KinectService::KinectService(void)
{
	m_hNextSkeletonEvent = NULL;

	m_hThNuiProcess = NULL;
	m_hEvNuiProcessStop = NULL;

	m_pSysMemSkeletonBuffer = NULL;
}
KinectService::~KinectService(void)
{ }
bool KinectService::Initialize()
{
	HRESULT hr;

	m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hr = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
		NUI_INITIALIZE_FLAG_USES_SKELETON |
		NUI_INITIALIZE_FLAG_USES_COLOR);

	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to initialize NUI library.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	hr = NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);

	if (FAILED(hr)) {
		MessageBox(0, L"Failed to open skeletal stream.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	// Start the Nui processing thread
	m_hEvNuiProcessStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThNuiProcess = CreateThread(NULL, 0, Nui_ProcessThread, this, 0, NULL);
	return true;
}
DWORD WINAPI KinectService::Nui_ProcessThread(LPVOID pParam)
{
	KinectService *pthis = (KinectService*)pParam;
	HANDLE hEvents[2];
	int nEventIdx;

	// Configure events to be listened on
	hEvents[0] = pthis->m_hEvNuiProcessStop;
	hEvents[1] = pthis->m_hNextSkeletonEvent;

	// Main thread loop
	while (1)
	{
		// Wait for an event to be signalled
		nEventIdx = WaitForMultipleObjects(sizeof(hEvents) / sizeof(hEvents[0]), hEvents, FALSE, 100);

		// If the stop event, stop looping and exit
		if (nEventIdx == 0)
			break;

		// Process signal event
		if (nEventIdx == 1)
			pthis->Nui_GotSkeletonAlert();
	}

	return (0);
}
void KinectService::Shutdown()
{
	// Stop the Nui processing thread
	if (m_hEvNuiProcessStop != NULL)
	{
		// Signal the thread
		SetEvent(m_hEvNuiProcessStop);

		// Wait for thread to stop
		if (m_hThNuiProcess != NULL)
		{
			WaitForSingleObject(m_hThNuiProcess, INFINITE);
			CloseHandle(m_hThNuiProcess);
		}
		CloseHandle(m_hEvNuiProcessStop);
	}

	NuiShutdown();

	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
		m_hNextSkeletonEvent = NULL;
	}
}
void KinectService::Nui_GotSkeletonAlert()
{
	NUI_SKELETON_FRAME SkeletonFrame;
	HRESULT hr = NuiSkeletonGetNextFrame(0, &SkeletonFrame);

	bool bFoundSkeleton = false;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
		if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
			bFoundSkeleton = true;

	if (!bFoundSkeleton)
		return;

	bool bBlank = true;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
		if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED && m_pSysMemSkeletonBuffer != NULL)
		{
			memcpy(m_pSysMemSkeletonBuffer, &SkeletonFrame, sizeof(NUI_SKELETON_FRAME));
			m_pSysMemSkeletonBuffer = NULL;
		}
}