#pragma once

#include "mini_sceneGraph.h"
#include "mini_camera.h"
#include "in_texturedEffect.h"
#include "mini_dxApplication.h"
#include "mini_collision.h"
#include "FW1\FW1FontWrapper.h"
#include "mini_material.h"
#include "mini_fpsCounter.h"

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
	};
}