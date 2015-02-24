#include "in_texturedEffect.h"

using namespace std;
using namespace mini;
using namespace DirectX;

TexturedEffect::TexturedEffect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps,
							   unique_ptr<ConstantBuffer<XMFLOAT4X4>>& cbProj,
							   unique_ptr<ConstantBuffer<XMFLOAT4X4>>& cbView,
							   unique_ptr<ConstantBuffer<XMFLOAT4X4, 2> >& cbModel,
							   unique_ptr<ConstantBuffer<Material::MaterialData>>& cbMaterial)
							   : Effect(std::move(vs), std::move(ps)), m_cbProj(cbProj), m_cbView(cbView), m_cbModel(cbModel), m_cbMaterial(cbMaterial)
{ 
}


void TexturedEffect::Begin(const dx_ptr<ID3D11DeviceContext>& context)
{
	Effect::Begin(context);
	ID3D11Buffer* vsb[3] = { m_cbProj->getBufferObject().get(), m_cbView->getBufferObject().get(), m_cbModel->getBufferObject().get() };
	context->VSSetConstantBuffers(0, 3, vsb);
	ID3D11Buffer* psb[1] = { m_cbMaterial->getBufferObject().get() };
	context->PSSetConstantBuffers(0, 1, psb);
}