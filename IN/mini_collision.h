#pragma once

#include <DirectXMath.h>
#include <vector>

namespace mini
{
	class OrientedBoundingRectangle;

	class BoundingCircle
	{
	public:
		/**********************************************************************
		Creates a bounding circle with given center and radius.
		**********************************************************************/
		BoundingCircle(DirectX::XMFLOAT2 center, float radius)
			: m_radius(radius), m_center(center) { }
		BoundingCircle(float cx, float cy, float radius)
			: m_radius(radius), m_center(cx, cy) { }
		BoundingCircle(const BoundingCircle& right)
			: m_radius(right.m_radius), m_center(right.m_center) { }

		inline float getRadius() const { return m_radius; }
		inline void setRadius(float radius) { m_radius = radius; }

		inline DirectX::XMFLOAT2 getCenter() const { return m_center; }
		inline void setCenter(DirectX::XMFLOAT2 center) { m_center = center; }
		inline void setCenter(float cx, float cy)
		{ m_center = DirectX::XMFLOAT2(cx, cy); }

	private:
		DirectX::XMFLOAT2 m_center;
		float m_radius;
	};

	class OrientedBoundingRectangle //oriented boudning rectangle
	{
	public:
		/**********************************************************************
		 Creates an oriented bounding rectangle  with given lower left corner,
		 width, height, and a rotation angle in radians.
		**********************************************************************/
		OrientedBoundingRectangle(DirectX::XMFLOAT2 corner = DirectX::XMFLOAT2(), float width = 0,
			float height = 0, float rotation = 0);
		OrientedBoundingRectangle(const OrientedBoundingRectangle& right);

		/**********************************************************************
		 Tests for  collision between this  bounding rectangle and  a bounding
		 circle. If  a collision is found  returns a translation  vector which
		 should be applied to the bounding circle to resolve the collision.
		**********************************************************************/
		DirectX::XMFLOAT2 Collision(const BoundingCircle& c);
		float Distance(const BoundingCircle& c);
		DirectX::XMFLOAT2 getP1() { return m_corner; }
		DirectX::XMFLOAT2 getP2() 
		{ return DirectX::XMFLOAT2(m_corner.x + m_dx.x, m_corner.y + m_dx.y); }
		DirectX::XMFLOAT2 getP3() 
		{ return DirectX::XMFLOAT2(m_corner.x + m_dx.x + m_dy.x, m_corner.y + m_dx.y + m_dy.y); }
		DirectX::XMFLOAT2 getP4() 
		{ return DirectX::XMFLOAT2(m_corner.x + m_dy.x, m_corner.y + m_dy.y); }

	private:
		DirectX::XMFLOAT2 m_corner;
		DirectX::XMFLOAT2 m_dx;
		DirectX::XMFLOAT2 m_dy;
		bool _Collision(DirectX::FXMVECTOR center, DirectX::XMVECTOR& trDir, float& minDist);
	};

	class CollisionEngine
	{
	public:
		CollisionEngine(DirectX::XMFLOAT2 characterPos = DirectX::XMFLOAT2(0, 0), float characterRadius = 0.3);
		DirectX::XMFLOAT2 SetObstacles(std::vector<mini::OrientedBoundingRectangle>&& obstacles);
		/**********************************************************************
		 Changes one of  the obstacle s and checks  for collistion.  If one is
		 found returns  a translation  vector which  should be applied  to the
		 character to resolve the collision.
		**********************************************************************/
		DirectX::XMFLOAT2 MoveObstacle(unsigned int obstacleIndex, mini::OrientedBoundingRectangle obstacle);
		/**********************************************************************
		 Applies translation  vector to the character  position and checks for
		 collisions.  If one is found,  the parameter is  changed to represent
		 the acctual  translation that  should be applied  to the character to 
		 avoid collisions, and returns the new position of the character.
		**********************************************************************/
		DirectX::XMFLOAT2 MoveCharacter(DirectX::XMFLOAT2& v);

		mini::OrientedBoundingRectangle getObstacle(unsigned int obstacleIndex);
		float DistanceToObstacle(unsigned int obstacleIdx);

	private:
		DirectX::XMFLOAT2 ResolveCollisions();
		std::vector<mini::OrientedBoundingRectangle> m_obstacles;
		mini::BoundingCircle m_character;
	};
}