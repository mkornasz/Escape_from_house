#pragma once
#pragma warning( push )
#pragma warning( disable : 4005)
#include <D3DX11.h>
#pragma warning( pop ) 

namespace mini
{
	namespace utils
	{
		/**********************************************************************
		 Wraper for  DXGI_SWAP_CHAIN_DESC  structure. Provides  initialization
		 of members with default values.
		**********************************************************************/
		struct SwapChainDescription : DXGI_SWAP_CHAIN_DESC
		{
			SwapChainDescription(HWND hWnd, UINT width = 0, UINT height = 0);
		};

		struct Texture2DDescription : D3D11_TEXTURE2D_DESC
		{
			Texture2DDescription(UINT width = 0, UINT height = 0);

			static Texture2DDescription DepthTextureDescription(UINT width = 0, UINT height = 0);
		};

		struct DepthViewDescription : D3D11_DEPTH_STENCIL_VIEW_DESC
		{
			DepthViewDescription();
		};

		struct ViewportDescription : D3D11_VIEWPORT
		{
			ViewportDescription(UINT width, UINT height);
		};

		struct BufferDescription : D3D11_BUFFER_DESC
		{
			BufferDescription(UINT byteWidth, D3D11_BIND_FLAG flags, D3D11_USAGE usage = D3D11_USAGE_DEFAULT);
		};

		struct RasterizerDescription : D3D11_RASTERIZER_DESC
		{
			RasterizerDescription();
		};

		struct SamplerDescription : D3D11_SAMPLER_DESC
		{
			SamplerDescription();
		};
	}
}