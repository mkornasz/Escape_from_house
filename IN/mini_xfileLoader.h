#include "mini_mesh.h"
#include "mini_xfileToken.h"
#include "mini_sceneNode.h"
#include "mini_dxDevice.h"
#include "mini_material.h"
#include <string>
#include <iostream>
#include <map>
#include <DirectXMath.h>

namespace mini
{
	class XFileLoader
	{
	public:
		XFileLoader(mini::DxDevice& d) : m_device(d) { }

		std::vector<mini::Mesh> m_meshes;
		std::vector<mini::SceneNode> m_nodes;
		std::vector<mini::Material> m_materials;

		void Load(std::string filename);
	private:
		void CheckHeader(std::istream& s);
		int ReadSceneNode(std::istream& s, int prevSybling);
		int ReadMesh(std::istream& s);
		void ReadMeshNormals(std::istream&s, std::vector<DirectX::XMFLOAT3>& normals, std::vector<unsigned short>& nindices);
		void ReadTexCoords(std::istream& s, std::vector<DirectX::XMFLOAT2>& texCoords);
		int ReadMeshMaterials(std::istream& s);
		int ReadMaterial(std::istream& s);
		std::string ReadTextureFilename(std::istream&s);
		int CreateMesh(std::vector<DirectX::XMFLOAT3>& positions, std::vector<DirectX::XMFLOAT2>& texCoords, std::vector<unsigned short>& indices, std::vector<DirectX::XMFLOAT3>& normals, std::vector<unsigned short>& nindices, unsigned int materiaIdx);
		int CreateMaterial(const std::string& materialName, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, const std::string& textureFilename);
		DirectX::XMFLOAT4X4 ReadFaceTransform(std::istream& s);
		void SkipDataObject(std::istream& s);
		void SkipDataReference(std::istream& s);

		template<class T>
		void ReadData(std::istream& s, T& d);

		template<class T>
		void ReadMember(std::istream& s, T& d);

		template<class iter>
		void ReadArray(std::istream& s, iter begin, iter end);

		void ReadFaceArray(std::istream& s, std::vector<unsigned short>& indices);

		void SkipToken(std::istream& s, XFileToken::Type type);

		mini::DxDevice& m_device;
		std::map<std::string, unsigned int> m_materialLookup;
	};
}