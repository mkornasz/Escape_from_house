#include "kinect_service.h"
#include <FaceTrackLib.h>
using namespace mini;

KinectService::KinectService(void)
{
	m_skeletonEvent = NULL;
	m_speechEvent = NULL;

	m_depthStreamHandle = NULL;
	m_videoStreamHandle = NULL;

	m_nuiProcess = NULL;
	m_nuiProcessStop = NULL;

	m_skeletonBuffer = NULL;

	pFTResult = NULL;
	pFaceTracker = NULL;
}
KinectService::~KinectService(void)
{ }
bool KinectService::Initialize()
{
	HRESULT hr;
	// Setup Kinectconst
	if (FAILED(NuiCreateSensorByIndex(0, &m_nuiSensor)))
	{
		MessageBox(0, L"Could not initialize the Kinect device.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	// Initialize NUI
	if (FAILED(NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_AUDIO | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH)))
	{
		MessageBox(0, L"Failed to initialize NUI library.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	// Initialize Skeleton events
	m_skeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (FAILED(NuiSkeletonTrackingEnable(m_skeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE)))
	{
		MessageBox(0, L"Failed to open skeletal stream.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	m_videoFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (FAILED(NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, m_videoFrameEvent, &m_videoStreamHandle)))
	{
		MessageBox(0, L"Failed to open image stream.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	m_depthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (FAILED(NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_320x240, 0, 2, m_depthFrameEvent, &m_depthStreamHandle)))
	{
		MessageBox(0, L"Failed to open image stream.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	// Start the Nui processing thread
	m_nuiProcessStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_nuiProcess = CreateThread(NULL, 0, Nui_ProcessThread, this, 0, NULL);
	
	initFaceTracker();

	// Initialize Audio
	if (FAILED(InitializeAudioStream()))
	{
		MessageBox(0, L"Failed to open audio stream.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	if (FAILED(CreateSpeechRecognizer()))
	{
		MessageBox(0, L"Failed to create speech recogniser.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	if (FAILED(LoadSpeechGrammar()))
	{
		MessageBox(0, L"Failed to load speech grammar.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}

	if (FAILED(StartSpeechRecognition()))
	{
		MessageBox(0, L"Failed to start speech recognition.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return false;
	}
	return true;
}
DWORD WINAPI KinectService::Nui_ProcessThread(LPVOID pParam)
{
	KinectService *pthis = (KinectService*)pParam;
	HANDLE hEvents[4];
	int nEventIdx;

	// Configure events to be listened on
	hEvents[0] = pthis->m_nuiProcessStop;
	hEvents[1] = pthis->m_depthFrameEvent;
	hEvents[2] = pthis->m_videoFrameEvent;
	hEvents[3] = pthis->m_skeletonEvent;

	// Main thread loop
	while (1)
	{
		//// Wait for an event to be signalled
		WaitForMultipleObjects(sizeof(hEvents) / sizeof(hEvents[0]), hEvents, FALSE, 100);

		// If the stop event is set, stop looping and exit
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_nuiProcessStop, 0))
		{
			break;
		}

		// Process signal events
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_depthFrameEvent, 0))
		{
			//pthis->GotDepthAlert();
			//pthis->m_FramesTotal++;
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_videoFrameEvent, 0))
		{
			//pthis->GotVideoAlert();
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_skeletonEvent, 0))
		{
				pthis->Nui_GotSkeletonAlert();
				pthis->storeFace();
		}
	}

	return (0);
}

void KinectService::Shutdown()
{
	// Stop the Nui processing thread
	if (m_nuiProcessStop != NULL)
	{
		// Signal the thread
		SetEvent(m_nuiProcessStop);

		// Wait for thread to stop
		if (m_nuiProcess != NULL)
		{
			WaitForSingleObject(m_nuiProcess, INFINITE);
			CloseHandle(m_nuiProcess);
		}
		CloseHandle(m_nuiProcessStop);
	}

	NuiShutdown();

	if (m_skeletonEvent && (m_skeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_skeletonEvent);
		m_skeletonEvent = NULL;
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
		if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED && m_skeletonBuffer != NULL)
		{
			memcpy(m_skeletonBuffer, &SkeletonFrame, sizeof(NUI_SKELETON_FRAME));
			m_skeletonBuffer = NULL;
		}
}
void KinectService::SetSysMemSkeletonBuffer(BYTE* buffer)
{
	m_skeletonBuffer = buffer;
}
BYTE* KinectService::GetSysMemSkeletonBuffer()
{
	return(m_skeletonBuffer);
}
HRESULT KinectService::InitializeAudioStream()
{
	INuiAudioBeam*      pNuiAudioSource = NULL;
	IMediaObject*       pDMO = NULL;
	IPropertyStore*     pPropertyStore = NULL;
	IStream*            pStream = NULL;

	// Get the audio source
	HRESULT hr = m_nuiSensor->NuiGetAudioSource(&pNuiAudioSource);
	if (SUCCEEDED(hr))
	{
		hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

		if (SUCCEEDED(hr))
		{
			pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);

			// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
			// Possible values are:
			//   SINGLE_CHANNEL_AEC = 0
			//   OPTIBEAM_ARRAY_ONLY = 2
			//   OPTIBEAM_ARRAY_AND_AEC = 4
			//   SINGLE_CHANNEL_NSAGC = 5
			PROPVARIANT pvSysMode;
			PropVariantInit(&pvSysMode);
			pvSysMode.vt = VT_I4;
			pvSysMode.lVal = (LONG)(4); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
			pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
			PropVariantClear(&pvSysMode);

			// Set DMO output format
			WAVEFORMATEX wfxOut = { WAVE_FORMAT_PCM /* Audio format */, 1 /* AudioChannels */, 16000 /* AudioSamplesPerSecond */
				, 32000 /* AudioAverageBytesPerSecond */, 2 /* AudioBlockAlign */, 16 /* AudioBitsPerSample */, 0 };
			DMO_MEDIA_TYPE mt = { 0 };
			MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

			mt.majortype = MEDIATYPE_Audio;
			mt.subtype = MEDIASUBTYPE_PCM;
			mt.lSampleSize = 0;
			mt.bFixedSizeSamples = TRUE;
			mt.bTemporalCompression = FALSE;
			mt.formattype = FORMAT_WaveFormatEx;
			memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

			hr = pDMO->SetOutputType(0, &mt, 0);

			if (SUCCEEDED(hr))
			{
				m_kinectAudioStream = new KinectAudioStream(pDMO);

				hr = m_kinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

				if (SUCCEEDED(hr))
				{
					hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_speechStream);
				}
			}

			MoFreeMediaType(&mt);
		}
	}

	pStream = NULL;
	pPropertyStore = NULL;
	pDMO = NULL;
	pNuiAudioSource = NULL;
	return hr;
}

HRESULT KinectService::StartSpeechRecognition()
{
	HRESULT hr = m_kinectAudioStream->StartCapture();

	if (SUCCEEDED(hr))
	{
		// Specify that all top level rules in grammar are now active
		m_speechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

		// Specify that engine should always be reading audio
		m_speechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

		// Specify that we're only interested in receiving recognition events
		m_speechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

		// Ensure that engine is recognizing speech and not in paused state
		m_speechContext->Pause(0);
		hr = m_speechContext->Resume(0);
		if (SUCCEEDED(hr))
		{
			m_speechEvent = m_speechContext->GetNotifyEventHandle();
		}
	}

	return hr;
}
HRESULT KinectService::LoadSpeechGrammar()
{
	HRESULT hr = m_speechContext->CreateGrammar(1, &m_speechGrammar);

	if (SUCCEEDED(hr))
	{
		// Populate recognition grammar from file
		LPCWSTR file = L"data/en-US.grxml";
		hr = m_speechGrammar->LoadCmdFromFile(file, SPLO_STATIC);
	}

	return hr;
}
HRESULT KinectService::CreateSpeechRecognizer()
{
	ISpObjectToken *pEngineToken = NULL;

	HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_speechRecognizer);

	if (SUCCEEDED(hr))
	{
		m_speechRecognizer->SetInput(m_speechStream, FALSE);
		hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"Language=409", NULL, &pEngineToken);

		if (hr == S_OK)
			printf("A");
		if (hr == REGDB_E_CLASSNOTREG)
			printf("B");
		if (hr == CLASS_E_NOAGGREGATION)
			printf("C");
		if (hr == SPERR_NOT_FOUND)
			printf("B");

		if (SUCCEEDED(hr))
		{
			m_speechRecognizer->SetRecognizer(pEngineToken);
			hr = m_speechRecognizer->CreateRecoContext(&m_speechContext);
		}
	}
	pEngineToken = NULL;
	return hr;
}

float* KinectService::GetFaceBuffers()
{
	return faceR;
}

void KinectService::initFaceTracker()
{
	HRESULT hr;
	pFaceTracker = FTCreateFaceTracker();	// We don't use any options.
	if (!pFaceTracker)
	{
		MessageBox(0, L"Could not create the face tracker.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return;
	}

	FT_CAMERA_CONFIG videoConfig;
	videoConfig.Width = 640;
	videoConfig.Height = 480;
	videoConfig.FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;			// 640x480

	FT_CAMERA_CONFIG depthConfig;
	depthConfig.Width = 320;
	depthConfig.Height = 240;
	depthConfig.FocalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	
	hr = pFaceTracker->Initialize(&videoConfig, &depthConfig, NULL, NULL);
	if (!pFaceTracker)
	{
		MessageBox(0, L"Could not create the face tracker.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return;
	}

	hr = pFaceTracker->CreateFTResult(&pFTResult);
	if (FAILED(hr) || !pFTResult)
	{
		if (hr == FT_ERROR_UNINITIALIZED)
			printf("S");
		else if (hr == E_POINTER)
			printf("S");
		MessageBox(0, L"Could not create the face tracker.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return;
	}

	m_imageBuffer = FTCreateImage();
	if (!m_imageBuffer || FAILED(hr = m_imageBuffer->Allocate(videoConfig.Width, videoConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
	{
		MessageBox(0, L"Could not create the color image.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return;
	}

	m_depthBuffer = FTCreateImage();
	if (!m_depthBuffer || FAILED(hr = m_depthBuffer->Allocate(320, 240, FTIMAGEFORMAT_UINT16_D13P3)))
	{
		MessageBox(0, L"Could not create the depth image.", L"Error", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		return;
	}

	lastTrackSucceeded = false;
}

void KinectService::storeFace()
{
	HRESULT hrFT = E_FAIL;
	FT_SENSOR_DATA sensorData(m_imageBuffer, m_depthBuffer);

	if (lastTrackSucceeded)
		hrFT = pFaceTracker->ContinueTracking(&sensorData, NULL, pFTResult);
	else
		hrFT = pFaceTracker->StartTracking(&sensorData, NULL, NULL, pFTResult);

	lastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(pFTResult->GetStatus());
	if (lastTrackSucceeded)
		pFTResult->Get3DPose(&faceScale, faceR, faceT);
	else
		pFTResult->Reset();
}
