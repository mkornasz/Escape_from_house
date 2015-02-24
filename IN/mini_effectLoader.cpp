#include "mini_effectLoader.h"

using namespace mini;
using namespace std;

EffectLoader::EffectLoader(DxDevice& device)
	: m_device(device)
{ }

void EffectLoader::Load(wstring filename)
{
	m_vsCode = m_device.CompileShader(filename, "VS_Main", "vs_4_0");
	m_psCode = m_device.CompileShader(filename, "PS_Main", "ps_4_0");
	m_vs = m_device.CreateVertexShader(m_vsCode);
	m_ps = m_device.CreatePixelShader(m_psCode);
}