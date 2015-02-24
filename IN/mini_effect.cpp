#include "mini_effect.h"

using namespace mini;

Effect::Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps)
	: m_vs(move(vs)), m_ps(move(ps))
{
}

void Effect::Begin(const dx_ptr<ID3D11DeviceContext>& context)
{
	context->VSSetShader(m_vs.get(), nullptr, 0);
	context->PSSetShader(m_ps.get(), nullptr, 0);
}