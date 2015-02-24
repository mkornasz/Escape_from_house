#include "mini_dxDevice.h"
#include "mini_exceptions.h"
#include <cassert>
#include <D3Dcompiler.h>

using namespace std;
using namespace mini;
using namespace mini::utils;

void DxDevice::Initialize(ID3D11Device* device)
{
	m_device.reset(device);
}

void DxDevice::Release()
{
	m_device.reset();
}

dx_ptr<ID3D11Texture2D> DxDevice::CreateTexture(const Texture2DDescription& desc)
{
	assert(m_device);
	ID3D11Texture2D* t = nullptr;
	HRESULT result = m_device->CreateTexture2D(&desc, nullptr, &t);
	dx_ptr<ID3D11Texture2D> texture(t);
	if (FAILED(result))
		THROW_DX11(result);
	return texture;
}

dx_ptr<ID3D11RenderTargetView> DxDevice::CreateRenderTargetView(const dx_ptr<ID3D11Texture2D>& backTexture)
{
	assert(m_device);
	ID3D11RenderTargetView* bb = nullptr;
	HRESULT result = m_device->CreateRenderTargetView(backTexture.get(), 0, &bb);
	dx_ptr<ID3D11RenderTargetView> backBuffer(bb);
	if (FAILED(result))
		THROW_DX11(result);
	return backBuffer;
}

dx_ptr<ID3D11DepthStencilView> DxDevice::CreateDepthStencilView(const dx_ptr<ID3D11Texture2D>& depthTexture)
{
	assert(m_device);
	ID3D11DepthStencilView* dsv = nullptr;
	HRESULT result = m_device->CreateDepthStencilView(depthTexture.get(), nullptr, &dsv);
	dx_ptr<ID3D11DepthStencilView> depthStencilView(dsv);
	if (FAILED(result))
		THROW_DX11(result);
	return depthStencilView;
}

dx_ptr<ID3D11Buffer> DxDevice::_CreateBuffer(const void * data, unsigned int byteLenght, D3D11_BIND_FLAG flag,
			D3D11_USAGE usage)
{
	BufferDescription desc(byteLenght, flag, usage);
	if ((usage & D3D11_USAGE_DYNAMIC) != 0 || (usage & D3D11_USAGE_STAGING) != 0)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return CreateBuffer(desc, data);
}

dx_ptr<ID3D11Buffer> DxDevice::CreateBuffer(const BufferDescription& desc, const void* pData)
{
	assert(m_device);
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = pData;
	ID3D11Buffer* b = nullptr;
	HRESULT result = m_device->CreateBuffer(&desc, pData ? &data : nullptr, &b);
	dx_ptr<ID3D11Buffer> buffer(b);
	if (FAILED(result))
		THROW_DX11(result);
	return buffer;
}

dx_ptr<ID3DBlob> DxDevice::CompileShader(const wstring& filePath, const string& entry, const string& shaderModel)
{
	assert(m_device);
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* eb = nullptr, *b = nullptr;
	HRESULT result = D3DX11CompileFromFile(filePath.c_str(), 0, 0, entry.c_str(), shaderModel.c_str(), shaderFlags,
		0, 0, &b, &eb, 0);
	dx_ptr<ID3DBlob> errorBuffer(eb);
	dx_ptr<ID3DBlob> buffer(b);
	if (FAILED(result))
	{
		if (errorBuffer)
		{
			char* msg = reinterpret_cast<char*>(errorBuffer->GetBufferPointer());
			OutputDebugStringA(msg);
		}
		THROW_DX11(result);
	}
	return buffer;
}

dx_ptr<ID3D11InputLayout> DxDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
	unsigned int layoutElements,
	dx_ptr<ID3DBlob>& vsByteCode)
{
	assert(m_device);
	ID3D11InputLayout* il = nullptr;
	HRESULT result = m_device->CreateInputLayout(layout, layoutElements, vsByteCode->GetBufferPointer(),
		vsByteCode->GetBufferSize(), &il);
	dx_ptr<ID3D11InputLayout> inputLayout(il);
	if (FAILED(result))
		THROW_DX11(result);
	return inputLayout;
}

dx_ptr<ID3D11VertexShader> DxDevice::CreateVertexShader(const dx_ptr<ID3DBlob>& byteCode)
{
	assert(m_device);
	ID3D11VertexShader* vs = nullptr;
	HRESULT result = m_device->CreateVertexShader(byteCode->GetBufferPointer(),
														byteCode->GetBufferSize(), 0, &vs);
	dx_ptr<ID3D11VertexShader> vertexShader(vs);
	if (FAILED(result))
		THROW_DX11(result);
	return vertexShader;
}

dx_ptr<ID3D11PixelShader> DxDevice::CreatePixelShader(const dx_ptr<ID3DBlob>& byteCode)
{
	assert(m_device);
	ID3D11PixelShader* ps = nullptr;
	HRESULT result = m_device->CreatePixelShader(byteCode->GetBufferPointer(),
													   byteCode->GetBufferSize(), 0, &ps);
	dx_ptr<ID3D11PixelShader> pixelShader(ps);
	if (FAILED(result))
		THROW_DX11(result);
	return pixelShader;
}

dx_ptr<ID3D11RasterizerState> DxDevice::CreateRasterizerState(const RasterizerDescription& desc)
{
	assert(m_device);
	ID3D11RasterizerState* s = nullptr;
	HRESULT result = m_device->CreateRasterizerState(&desc, &s);
	dx_ptr<ID3D11RasterizerState> state(s);
	if (FAILED(result))
		THROW_DX11(result);
	return state;
}

dx_ptr<ID3D11SamplerState> DxDevice::CreateSamplerState(const SamplerDescription& desc)
{
	assert(m_device);
	ID3D11SamplerState* s = nullptr;
	HRESULT result = m_device->CreateSamplerState(&desc, &s);
	dx_ptr<ID3D11SamplerState> sampler(s);
	if (FAILED(result))
		THROW_DX11(result);
	return sampler;
}

dx_ptr<ID3D11ShaderResourceView> DxDevice::CreateShaderResourceView(const wstring& texFilePath)
{
	assert(m_device);
	ID3D11ShaderResourceView* rv = nullptr;
	HRESULT result = D3DX11CreateShaderResourceViewFromFile(m_device.get(), texFilePath.c_str(), 0, 0, &rv, 0);
	dx_ptr<ID3D11ShaderResourceView> resourceView(rv);
	if (FAILED(result))
		THROW_DX11(result);
	return resourceView;
}
