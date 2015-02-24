#include "mini_mesh.h"
#include <algorithm>
using namespace std;
using namespace mini;

void Mesh::Render(const dx_ptr<ID3D11DeviceContext>& context)
{
	if (!m_indexBuffer || !m_vertexBuffers)
		return;
	context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, m_buffersCount, m_vertexBuffers.get(), m_strides.get(), m_offsets.get());
	context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::Mesh()
	: m_buffersCount(0), m_indexCount(0), m_materialIdx(UINT_MAX)
{ }

Mesh::Mesh(vector<dx_ptr<ID3D11Buffer>>&& vbuffers, vector<unsigned int>&& vstrides, vector<unsigned int>&& voffsets,
		   dx_ptr<ID3D11Buffer>&& indices, unsigned int indexCount, unsigned int materialIdx)
{
	m_indexCount = indexCount;
	m_buffersCount = vbuffers.size();
	m_indexBuffer = move(indices);
	m_materialIdx = materialIdx;
	
	m_vertexBuffers.reset(new ID3D11Buffer*[m_buffersCount]);
	/*for ( int i = 0; i < m_buffersCount; ++i )
		m_vertexBuffers[i] = vbuffers[i].release();*/
	transform(vbuffers.begin(), vbuffers.end(), &m_vertexBuffers[0], [](dx_ptr<ID3D11Buffer>& p){return p.release();});
	vbuffers.clear();

	m_strides.reset(new unsigned int[m_buffersCount]);
	copy(vstrides.begin(), vstrides.end(), &m_strides[0]);
	vstrides.clear();

	m_offsets.reset(new unsigned int[m_buffersCount]);
	copy(voffsets.begin(), voffsets.end(), &m_offsets[0]);
	voffsets.clear();
}

Mesh::~Mesh()
{
	Release();
}

void Mesh::Release()
{
	DxDeleter<ID3D11Buffer> d;
	if (m_vertexBuffers)
		for ( unsigned int i = 0; i < m_buffersCount; ++i)
			d(m_vertexBuffers[i]);
	m_vertexBuffers.reset();
	m_strides.reset();
	m_offsets.reset();
	m_indexBuffer.reset();
	m_buffersCount = 0;
	m_indexCount = 0;
	m_materialIdx = UINT_MAX;
}

Mesh& Mesh::operator=(Mesh&& right)
{
	Release();
	m_vertexBuffers = move(right.m_vertexBuffers);
	m_indexBuffer = move(right.m_indexBuffer);
	m_strides = move(right.m_strides);
	m_offsets = move(right.m_offsets);
	m_buffersCount = right.m_buffersCount;
	m_materialIdx = right.m_materialIdx;
	right.Release();
	return *this;
}

Mesh::Mesh(Mesh&& right)
	: m_vertexBuffers(move(right.m_vertexBuffers)), m_strides(move(right.m_strides)), m_offsets(move(right.m_offsets)),
	m_buffersCount(right.m_buffersCount), m_indexBuffer(move(right.m_indexBuffer)), m_indexCount(right.m_indexCount), m_materialIdx(right.m_materialIdx)
{
	right.Release();
}