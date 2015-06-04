# pragma once
#include "kinect_audioStream.h"

#include <Windows.h>
#include "NuiApi.h"

// Speech interface
#include <sapi.h>
#include <sphelper.h>

// Audio interface
#include <propsys.h>
#include <wmcodecdsp.h>
#include <uuids.h>
#include <dmo.h>
#include <mfobjects.h>

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
		static DWORD WINAPI Nui_ProcessThread(LPVOID pParam);
		HRESULT InitializeAudioStream();
		HRESULT LoadSpeechGrammar();
		HRESULT CreateSpeechRecognizer();
		HRESULT StartSpeechRecognition();

		INuiSensor *m_nuiSensor;
		HANDLE m_nuiProcess;
		HANDLE m_nuiProcessStop;
			   
		HANDLE m_skeletonEvent;
		BYTE* m_skeletonBuffer;
			   
		KinectAudioStream *m_kinectAudioStream;
		HANDLE m_speechEvent;
		ISpStream *m_speechStream;
		ISpRecoContext *m_speechContext;
		ISpRecoGrammar *m_speechGrammar;
		ISpRecognizer *m_speechRecognizer;
	};
}