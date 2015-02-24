#pragma once

#include "mini_dxDevice.h"

namespace mini
{
	class EffectLoader
	{
	public:
		EffectLoader(mini::DxDevice& device);

		void Load(std::wstring filename);

		dx_ptr<ID3DBlob> m_vsCode;
		dx_ptr<ID3DBlob> m_psCode;
		dx_ptr<ID3D11VertexShader> m_vs;
		dx_ptr<ID3D11PixelShader> m_ps;

	private:
		mini::DxDevice& m_device;
	};
}