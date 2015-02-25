#pragma once

#include "mini_sceneGraph.h"
#include "mini_camera.h"
#include "in_texturedEffect.h"
#include "mini_dxApplication.h"
#include "mini_collision.h"
#include "FW1\FW1FontWrapper.h"
#include "mini_material.h"
#include "mini_fpsCounter.h"

#include "dinput.h"

namespace mini
{
	class INScene : public mini::DxApplication
	{
	public:
		INScene(HINSTANCE hInstance,
			int wndWidth = mini::Window::m_defaultWindowWidth,
			int wndHeight = mini::Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"DirectX Window")
			: mini::DxApplication(hInstance, wndWidth, wndHeight, wndTitle), m_camera(DirectX::XMFLOAT3(0, 1.6f, 0))
		{ }

	protected:
		virtual bool Initialize();
		virtual void Render();
		virtual void Shutdown();
		virtual void Update(float dt);
		virtual bool ProcessMessage(mini::WindowMessage& msg);

	private:
		//Starts the door opening animation if the door isn't fully open
		void OpenDoor();
		//Starts the door closing animation if the door isn't fully closed
		void CloseDoor();
		//Toggles between the door opening and closing animation
		void ToggleDoor();
		//Moves the character forward by dx and right by dz in relation to the current camera orientation
		void MoveCharacter(float dx, float dz);
		//Checks if the camera is facing the door
		bool FacingDoor();
		//Returns the distance from the character to the door
		float DistanceToDoor();

		void InitializeInput();
		void UpdateDoor(float dt);
		void RenderText();

		// Handle mouse and keyboard changes using Windows API
		void HandleMouseChange(WindowMessage& msg);
		void HandleKeyboardChange(float dt);

		// Handle mouse and keyboard changes using DirectX API
		bool GetDeviceState(IDirectInputDevice8* pDevice, unsigned int size, void* ptr);
		void HandleMouseChangeDI();
		void HandleKeyboardChangeDI(float dt);

		//Initialize all variables
		void InitializeEnvironment();
		// Choose the available controler
		void ChooseControler(BYTE keyboardState[256]);
		//Render controler chooser menu
		void RenderControlerMenu();
		//Render keyboard chooser menu
		void RenderKeyboard();
		//Render joystick chooser menu
		void RenderJoystick();

		std::unique_ptr<ConstantBuffer<DirectX::XMFLOAT4X4>> m_cbProj;
		std::unique_ptr<ConstantBuffer<DirectX::XMFLOAT4X4>> m_cbView;
		std::unique_ptr<ConstantBuffer<DirectX::XMFLOAT4X4, 2> > m_cbModel;
		std::unique_ptr<ConstantBuffer<mini::Material::MaterialData>> m_cbMaterial;
		std::unique_ptr<mini::TexturedEffect> m_texturedEffect;
		std::unique_ptr<mini::SceneGraph> m_sceneGraph;
		mini::dx_ptr<ID3D11InputLayout> m_layout;
		mini::dx_ptr<ID3D11RasterizerState> m_rsState;
		mini::dx_ptr<ID3D11SamplerState> m_sampler;
		mini::dx_ptr<IFW1Factory> m_fontFactory;
		mini::dx_ptr<IFW1FontWrapper> m_font;
		mini::Camera m_camera;
		DirectX::XMFLOAT4X4 m_proj;
		mini::FPSCounter m_counter;

		unsigned int m_doorNode;
		float m_doorAngle;
		float m_doorAngVel;
		DirectX::XMFLOAT4X4 m_doorTransform;
		mini::CollisionEngine m_collisions;

		// Camera movement variables
		bool m_isMouseDown;
		bool m_isReturnDown;
		POINT m_lastMousePosition;

		// DirectX devices
		IDirectInput8 * di;
		IDirectInputDevice8* mouse;
		IDirectInputDevice8* keyboard;
		IDirectInputDevice8* joystick;

		// Device state constants
		static const unsigned int GET_STATE_RETRIES = 2;
		static const unsigned int ACQUIRE_RETRIES = 2;

		bool m_showControlers;
		int m_controlerNumber;
		int m_chosenControler;
		
		bool m_renderKeyboard;
		bool m_renderJoystick;
	};
}