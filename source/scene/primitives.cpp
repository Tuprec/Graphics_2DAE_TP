//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <primitives.h>
using namespace gfx;

//--- Primitives AABB Updates ---
void Sphere::UpdateAABB()
{
	const Vector3 min{ origin.x - radius ,origin.y - radius ,origin.z - radius };
	const Vector3 max{ origin.x + radius ,origin.y + radius ,origin.z + radius };
	bounding_box = AABB{ min,max };
}

void Plane::UpdateAABB()
{
	if (!half_extent.has_value())
	{
		return;
	}
	const Vector3 bitangent{ Vector3::Cross(normal, tangent) };

	const Vector2 extent{ half_extent.value() };

	const Vector3 world_extent{
		std::abs(tangent.x) * extent.x + std::abs(bitangent.x) * extent.y,
		std::abs(tangent.y) * extent.x + std::abs(bitangent.y) * extent.y,
		std::abs(tangent.z) * extent.x + std::abs(bitangent.z) * extent.y
	};

	const Vector3 min{ origin - world_extent };
	const Vector3 max{ origin + world_extent };

	bounding_box = AABB{ min, max };
}

void Triangle::UpdateAABB()
{
	AABB temp{};
	temp.ExpandToInclude(v0);
	temp.ExpandToInclude(v1);
	temp.ExpandToInclude(v2);
	bounding_box = temp;
}

void TriangleMesh::UpdateAABB()
{
	AABB temp{};
	for (Vertex v : vertices)
	{
		temp.ExpandToInclude(v.position);
	}
	bounding_box = temp;

}

//--- AABB ---
void AABB::ExpandToInclude(const Vector3& point)
{
	min = Vector3::Min(min, point);
	max = Vector3::Max(max, point);
}

void AABB::ExpandToInclude(const AABB& other)
{
	min = Vector3::Min(min, other.min);
	max = Vector3::Max(max, other.max);
}

bool AABB::Contains(const Vector3& point) const
{
	return point.x >= min.x && point.x <= max.x &&
		point.y >= min.y && point.y <= max.y &&
		point.z >= min.z && point.z <= max.z;
}

bool AABB::Overlaps(const AABB& other) const
{
	return min.x <= other.max.x && max.x >= other.min.x &&
		min.y <= other.max.y && max.y >= other.min.y &&
		min.z <= other.max.z && max.z >= other.min.z;
}

Vector3 AABB::GetCenter() const
{
	return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f };
}

Vector3 AABB::GetSize() const
{
	return { max.x - min.x, max.y - min.y, max.z - min.z };
}

bool AABB::IsValid() const
{
	return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}