#include "mini_dxstructures.h"
#include <memory>
using namespace mini;
using namespace mini::utils;

SwapChainDescription::SwapChainDescription(HWND hWnd, UINT width, UINT height)
{
	ZeroMemory(this, sizeof(SwapChainDescription));
	BufferCount = 1;
	BufferDesc.Width = width;
	BufferDesc.Height = height;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	BufferDesc.RefreshRate.Numerator = 120;
	BufferDesc.RefreshRate.Denominator = 1;
	BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	OutputWindow = hWnd;
	Windowed = true;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
}

Texture2DDescription::Texture2DDescription(UINT width, UINT height)
{
	ZeroMemory(this, sizeof(Texture2DDescription));
	Width = width;
	Height = height;
	MipLevels = 0;
	ArraySize = 1;
	Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
	Usage = D3D11_USAGE_DEFAULT;
	BindFlags = D3D11_BIND_SHADER_RESOURCE;
	CPUAccessFlags = 0;
	MiscFlags = 0;
}

Texture2DDescription Texture2DDescription::DepthTextureDescription(UINT width, UINT height)
{
	Texture2DDescription desc(width, height);
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	return desc;
}

DepthViewDescription::DepthViewDescription()
{
	ZeroMemory(this, sizeof(DepthViewDescription));
	Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	Flags = 0;
	Texture2D.MipSlice = 0;
}

ViewportDescription::ViewportDescription(UINT width, UINT height)
{
	Width = static_cast<float>(width);
	Height = static_cast<float>(height);
	MinDepth = 0.0f;
	MaxDepth = 1.0f;
	TopLeftX = 0.0f;
	TopLeftY = 0.0f;
}

BufferDescription::BufferDescription(UINT byteWidth, D3D11_BIND_FLAG flags, D3D11_USAGE usage)
{
	ZeroMemory(this, sizeof(BufferDescription));
	Usage = usage;
	BindFlags = flags;
	ByteWidth = byteWidth;
}

RasterizerDescription::RasterizerDescription()
{
	FillMode = D3D11_FILL_SOLID; //Determines the solid fill mode (as opposed to wireframe)
	CullMode = D3D11_CULL_BACK; //Indicates that back facing triangles are not drawn
	FrontCounterClockwise = false; //Indicates that vertices of a front facing triangle are counter-clockwise on the render target
	DepthBias = 0;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 0.0f;
	DepthClipEnable = true;
	ScissorEnable = false;
	MultisampleEnable = false;
	AntialiasedLineEnable = false;
}

SamplerDescription::SamplerDescription()
{
	Filter = D3D11_FILTER_ANISOTROPIC;
	AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	MinLOD = -D3D11_FLOAT32_MAX;
	MaxLOD = D3D11_FLOAT32_MAX;
	MipLODBias = 0.0f;
	MaxAnisotropy = 16;
	ComparisonFunc = D3D11_COMPARISON_NEVER;
	BorderColor[0] = 0.0f;
	BorderColor[1] = 0.0f;
	BorderColor[2] = 0.0f;
	BorderColor[3] = 0.0f;
}