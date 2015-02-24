#include "mini_collision.h"
#include <cfloat>


using namespace std;
using namespace mini;
using namespace DirectX;

OrientedBoundingRectangle::OrientedBoundingRectangle(DirectX::XMFLOAT2 corner,
	float width, float height, float rotation)
{
	XMVECTOR c = XMLoadFloat2(&corner);
	XMMATRIX m = 
		XMMatrixAffineTransformation2D(XMVectorSet(width, height, 0, 0),
			XMVectorZero(), rotation, c);

	XMVECTOR dx = XMVectorSet(1, 0, 0, 0);
	XMVECTOR dy = XMVectorSet(0, 1, 0, 0);
	dx = XMVector2TransformNormal(dx, m);
	dy = XMVector2TransformNormal(dy, m);
	if (width < 0 && height > 0)
	{
		c += dx;
		dx = -dx;
	}
	else if (width > 0 && height < 0)
	{
		c += dy;
		dy = -dy;
	}
	XMStoreFloat2(&m_dx, dx);
	XMStoreFloat2(&m_dy, dy);
	XMStoreFloat2(&m_corner, c);
}

OrientedBoundingRectangle::OrientedBoundingRectangle(const OrientedBoundingRectangle& right)
	: m_corner(right.m_corner), m_dx(right.m_dx), m_dy(right.m_dy)
{ }

static float clamp(float t, float min, float max)
{
	if (t < min)
		return min;
	if (t > max)
		return max;
	return t;
}

float OrientedBoundingRectangle::Distance(const BoundingCircle& circle)
{
	XMFLOAT2 cntr = circle.getCenter();
	XMVECTOR center = XMLoadFloat2(&cntr);
	XMVECTOR translate;
	float min_dist;
	bool inside = _Collision(center, translate, min_dist);
	return inside ? 0.0f : min_dist;
}

bool OrientedBoundingRectangle::_Collision(FXMVECTOR center, XMVECTOR& translate, float& min_dist)
{
	XMVECTOR p[4];
	XMVECTOR v[4];
	float len[4];
	p[0] = XMLoadFloat2(&m_corner);
	v[0] = XMLoadFloat2(&m_dx);
	v[1] = XMLoadFloat2(&m_dy);
	len[0] = len[2] = XMVectorGetX(XMVector2Length(v[0]));
	len[1] = len[3] = XMVectorGetX(XMVector2Length(v[1]));
	v[0] = XMVector2Normalize(v[0]);
	v[1] = XMVector2Normalize(v[1]);
	v[2] = -v[0];
	v[3] = -v[1];
	for (int i = 1; i < 4; ++i)
		p[i] = p[i - 1] + len[i - 1] * v[i - 1];

	bool inside = true;
	min_dist = FLT_MAX;
	translate = XMVectorZero();
	for (int i = 0; i < 4; ++i)
	{
		XMVECTOR q = center - p[i];
		float t = XMVectorGetX(XMVector2Dot(q, v[i]));
		float clampt = clamp(t, 0, len[i]);
		if (t != clampt)
			inside = false;
		q = p[i] + clampt*v[i];
		XMVECTOR trDir = center - q;
		float dist = XMVectorGetX(XMVector2Length(trDir));
		if (dist < min_dist)
		{
			min_dist = dist;
			translate = dist == 0 ? XMVector2Orthogonal(v[i]) 
				: XMVector2Normalize(trDir);
		}
	}
	return inside;
}

XMFLOAT2 OrientedBoundingRectangle::Collision(const BoundingCircle& circle)
{
	XMFLOAT2 cntr = circle.getCenter();
	XMVECTOR center = XMLoadFloat2(&cntr);
	XMVECTOR translate;
	float min_dist;
	bool inside = _Collision(center, translate, min_dist);
	float radius = circle.getRadius();
	if (inside)
		translate = -translate*(min_dist + radius);
	else if (min_dist < radius)
		translate *= radius - min_dist;
	else translate = XMVectorZero();
	XMFLOAT2 tr;
	XMStoreFloat2(&tr, translate);
	return tr;
}

CollisionEngine::CollisionEngine(XMFLOAT2 characterPosition, float characterRadius)
	: m_character(characterPosition, characterRadius)
{ }

XMFLOAT2 CollisionEngine::SetObstacles(vector<OrientedBoundingRectangle>&& obstacles)
{
	m_obstacles = move(obstacles);
	return ResolveCollisions();
}

XMFLOAT2 CollisionEngine::MoveObstacle(unsigned int obstacleIndex, OrientedBoundingRectangle obstacle)
{
	if (obstacleIndex >= m_obstacles.size())
		return XMFLOAT2();
	m_obstacles[obstacleIndex] = obstacle;
	return ResolveCollisions();
}

XMFLOAT2 CollisionEngine::MoveCharacter(XMFLOAT2& v)
{
	XMFLOAT2 oldPos = m_character.getCenter();
	XMFLOAT2 newPos = XMFLOAT2(oldPos.x + v.x, oldPos.y + v.y);
	m_character.setCenter(newPos);
	ResolveCollisions();
	newPos = m_character.getCenter();
	v.x = newPos.x - oldPos.x;
	v.y = newPos.y - oldPos.y;
	return newPos;
}

XMFLOAT2 CollisionEngine::ResolveCollisions()
{
	XMFLOAT2 oldPos = m_character.getCenter();
	XMFLOAT2 newPos = oldPos;
	m_character.setCenter(newPos);
	for ( OrientedBoundingRectangle& rect : m_obstacles )
	{
		XMFLOAT2 tr = rect.Collision(m_character);
		newPos.x += tr.x;
		newPos.y += tr.y;
		m_character.setCenter(newPos);
	}
	XMFLOAT2 v;
	v.x = newPos.x - oldPos.x;
	v.y = newPos.y - oldPos.y;
	return v;
}

OrientedBoundingRectangle CollisionEngine::getObstacle(unsigned int obstacleIndex)
{
	if (obstacleIndex >= m_obstacles.size())
		return OrientedBoundingRectangle();
	return m_obstacles[obstacleIndex];
}

float CollisionEngine::DistanceToObstacle(unsigned int obstacleIndex)
{
	if (obstacleIndex >= m_obstacles.size())
		return FLT_MAX;
	return m_obstacles[obstacleIndex].Distance(m_character);
}