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

#define MAPENTRY(p) {p, #p}

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

		// Initialize all the variables
		void InitializeInput();
		void UpdateDoor(float dt);
		void RenderText();

		// Handle mouse changes using Windows API
		void HandleMouseChange(WindowMessage& msg);
		// Handle keyboard changes using Windows API
		void HandleKeyboardChange(float dt);

		// Get the state of the device using DirectX API
		bool GetDeviceState(IDirectInputDevice8* pDevice, unsigned int size, void* ptr);
		// Handle mouse changes using DirectX API
		void HandleMouseChangeDI();
		// Handle keyboard changes using DirectX API
		void HandleKeyboardChangeDI(float dt);
		// Handle joystick changes using DirectX API
		void HandleJoystickChangeDI(float dt);

		//Initialize all variables
		void InitializeEnvironment();
		// Choose the available controler
		void ChooseControler(BYTE keyboardState[256]);
		//Choose a button responsible for certain action
		void ChooseButton(BYTE keyboardState[256]);
		//Render controler chooser menu
		void RenderControlerMenu();
		//Renders Keyboard in the controler menu and returns true if it's available
		bool RenderControlerMenuKeyboard(int left, int top);
		//Renders Joystick in the controler menu and returns true if it's available
		bool RenderControlerMenuJoystick(int left, int top);
		//Render buttons chooser menu
		void RenderButtons();

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

		// DirectX input
		IDirectInput8 * di;
		// DirectX mouse device
		IDirectInputDevice8* mouse;
		// DirectX keyboard device
		IDirectInputDevice8* keyboard;
		// DirectX joystick device
		IDirectInputDevice8* joystick;
		// Keeps Joystick capabilities
		DIDEVCAPS m_capabilities;

		static const unsigned int GET_STATE_RETRIES = 2;
		static const unsigned int ACQUIRE_RETRIES = 2;
		static const unsigned int MaxAxisRange = 32767;

		//Should show available controlers
		bool m_showControlers;
		//Number of available controlers
		int m_controlerNumber;
		//Chosen controler (from enum)
		int m_chosenControler;
		//Highlighted index of the controler
		int m_highlitedIndex;
		//Maximum button index which can be set (for mouse we don't set camera rotation)
		int m_maxButtonIndex;

		//Should render buttons chooser menu
		bool m_renderButtons;

		//Keys corresponding to used buttons
		int m_buttons[6];
		//Names of the buttons
		char* m_buttonsStrings[6];

		//Available controlers
		enum Controlers
		{
			Keyboard, Joystick
		};
		//Available camera rotations
		enum CameraRotation
		{
			UpRotate, DownRotate, LeftRotate, RightRotate
		};
		//Available buttons
		enum Buttons
		{
			Up, Down, Left, Right, Return, Menu
		};
	};
}