#pragma once

#include <DirectXMath.h>

namespace mini
{
	class Camera
	{
	public:
		Camera(DirectX::XMFLOAT3 position);
		
		DirectX::XMMATRIX GetViewMatrix();
		void GetViewMatrix(DirectX::XMMATRIX& m);
		
		DirectX::XMFLOAT4 getPosition()
		{ return m_position; }
		DirectX::XMVECTOR getForwardDir();
		DirectX::XMVECTOR getRightDir();

		void Move(DirectX::XMFLOAT3 v);
		void Rotate(float dx, float dy);

	private:
		float m_angleX, m_angleY;
		DirectX::XMFLOAT4 m_position;
	};
}