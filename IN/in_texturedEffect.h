#pragma once

#include "mini_effect.h"
#include "mini_dxDevice.h"
#include "mini_constantBuffer.h"
#include "mini_material.h"

namespace mini
{
	class TexturedEffect : public mini::Effect
	{
	public:

		TexturedEffect(mini::dx_ptr<ID3D11VertexShader>&& vs, 
			mini::dx_ptr<ID3D11PixelShader>&& ps,
			std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4>>& cbProj,
			std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4>>& cbView,
			std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4, 2> >& cbModel,
			std::unique_ptr<mini::ConstantBuffer<mini::Material::MaterialData>>& cbMaterial
			);


		virtual void Begin(const mini::dx_ptr<ID3D11DeviceContext>& context);

	private:
		std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4>>& m_cbProj;
		std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4>>& m_cbView;
		std::unique_ptr<mini::ConstantBuffer<DirectX::XMFLOAT4X4, 2> >& m_cbModel;
		std::unique_ptr<mini::ConstantBuffer<mini::Material::MaterialData>>& m_cbMaterial;
	};
}