#pragma once

#include "mini_dxptr.h"
#include <DirectXMath.h>

namespace mini
{
	class Effect
	{
	public:
		Effect(mini::dx_ptr<ID3D11VertexShader>&& vs, mini::dx_ptr<ID3D11PixelShader>&& ps);
		virtual void Begin(const mini::dx_ptr<ID3D11DeviceContext>& context);
		
	private:
		mini::dx_ptr<ID3D11VertexShader> m_vs;
		mini::dx_ptr<ID3D11PixelShader> m_ps;
	};
}