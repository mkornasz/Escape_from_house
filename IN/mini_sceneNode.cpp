#include "mini_sceneNode.h"

using namespace mini;
using namespace DirectX;

SceneNode::SceneNode()
	: m_firstChild(-1), m_nextSybling(-1), m_parent(-1), m_mesh(-1)
{
	XMMATRIX m = XMMatrixIdentity();
	XMStoreFloat4x4(&m_transform, m);
	XMStoreFloat4x4(&m_localTransform, m);
}