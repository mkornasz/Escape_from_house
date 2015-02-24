#pragma once

#include "mini_dxptr.h"
#include "mini_dxstructures.h"
#include <vector>

namespace mini
{
	class DxDevice
	{
	public:
		void Initialize(ID3D11Device* device);
		void Release();

		mini::dx_ptr<ID3D11Texture2D> CreateTexture(const mini::utils::Texture2DDescription& desc);
		mini::dx_ptr<ID3D11RenderTargetView> CreateRenderTargetView(const mini::dx_ptr<ID3D11Texture2D>& backTexture);
		mini::dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const mini::dx_ptr<ID3D11Texture2D>& depthTexture);

		mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
															 unsigned int layoutElements,
															 mini::dx_ptr<ID3DBlob>& vsByteCode);

		template<class T>
		mini::dx_ptr<ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& data)
		{
			return _CreateBuffer(reinterpret_cast<const void*>(data.data()),
				data.size()*sizeof(T), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT);
		}

		mini::dx_ptr<ID3D11Buffer> CreateIndexBuffer(const std::vector<unsigned short>& indices)
		{
			return _CreateBuffer(reinterpret_cast<const void*>(indices.data()),
				sizeof(unsigned short) * indices.size(), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT);
		}

		mini::dx_ptr<ID3D11Buffer> CreateBuffer(const mini::utils::BufferDescription& desc, const void* pData = nullptr);

		mini::dx_ptr<ID3DBlob> CompileShader(const std::wstring& filePath, const std::string& entry,
			const std::string& shaderModel);
		mini::dx_ptr<ID3D11VertexShader> CreateVertexShader(const mini::dx_ptr<ID3DBlob>& byteCode);
		mini::dx_ptr<ID3D11PixelShader> CreatePixelShader(const mini::dx_ptr<ID3DBlob>& byteCode);
		mini::dx_ptr<ID3D11RasterizerState> CreateRasterizerState(const mini::utils::RasterizerDescription& desc);
		mini::dx_ptr<ID3D11SamplerState> CreateSamplerState(const mini::utils::SamplerDescription& desc);
		mini::dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const std::wstring& texFilePath);

		ID3D11Device * getDevicePtr() { return m_device.get(); }
	private:
		mini::dx_ptr<ID3D11Buffer> _CreateBuffer(const void * data, unsigned int byteLenght, D3D11_BIND_FLAG flag,
			D3D11_USAGE usage);

		mini::dx_ptr<ID3D11Device> m_device;
	};
}