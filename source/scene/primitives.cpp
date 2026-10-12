//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <primitives.h>
#include <vector>
#include <numeric>
#include <execution>

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

	const Vector2 extent{ half_extent.value() };


	const Vector3 min{ origin - Vector3{half_extent.value().x,half_extent.value().y,0} };
	const Vector3 max{ origin + Vector3{half_extent.value().x,half_extent.value().y,0} };

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
	const uint32_t triangle_count{ indices.size() / 3 };
	bvh_tri_indices.resize(triangle_count);
	std::iota(bvh_tri_indices.begin(), bvh_tri_indices.end(), 0);
	std::vector<AABB> aabbs{};
	std::vector<Vector3> centroids{};
	aabbs.reserve(triangle_count);
	centroids.reserve(triangle_count);


	for (uint32_t idx{}; idx < indices.size(); idx += 3)
	{
		const uint32_t i0{ indices[idx] };
		const uint32_t i1{ indices[idx + 1] };
		const uint32_t i2{ indices[idx + 2] };


		//Setting the vertexes of the triangle
		const Vector3 v0 = vertices[i0].position;
		const Vector3 v1 = vertices[i1].position;
		const Vector3 v2 = vertices[i2].position;

		aabbs.push_back(AABB{});

		const int32_t aabb_idx{ aabbs.size() };

		aabbs[aabb_idx].ExpandToInclude(v0);
		aabbs[aabb_idx].ExpandToInclude(v1);
		aabbs[aabb_idx].ExpandToInclude(v2);

		centroids.push_back(Vector3{
			(v0.x + v1.x + v2.x) / 3.f,
			(v0.y + v1.y + v2.y) / 3.f,
			(v0.z + v1.z + v2.z) / 3.f
			});
	}
	bvh_nodes.clear();

	if (!bvh_tri_indices.empty())
	{
		bvh_nodes.reserve(2 * bvh_tri_indices.size());

		BuildBVHNode(0, static_cast<uint32_t>(bvh_tri_indices.size()), aabbs, centroids);
	}
}

uint32_t TriangleMesh::BuildBVHNode(uint32_t begin, uint32_t end, const std::vector<AABB>& aabbs, const std::vector<Vector3>& centroids)
{
	constexpr uint32_t MAX_TRIANGLES_PER_LEAF{ 4 };

	const uint32_t node_idx{ static_cast<uint32_t>(bvh_nodes.size()) };

	bvh_nodes.emplace_back();

	AABB bounds{};

	for (uint32_t i{ begin }; i < end; ++i)
	{
		bounds.ExpandToInclude(aabbs[bvh_tri_indices[i]]);
	}

	bvh_nodes[node_idx].bounds = bounds;

	const uint32_t count{ end - begin };

	if (count <= MAX_TRIANGLES_PER_LEAF)
	{
		bvh_nodes[node_idx].first_tri = begin;
		bvh_nodes[node_idx].tri_count = count;
		return node_idx;
	}

	AABB centroid_bounds{};

	for (uint32_t i{ begin }; i < end; ++i)
	{
		centroid_bounds.ExpandToInclude(centroids[bvh_tri_indices[i]]);
	}

	const Vector3 size = centroid_bounds.GetSize();
	const Vector3 center = centroid_bounds.GetCenter();

	uint32_t axis = 0;

	if (size.y > size.x && size.y >= size.z)
	{
		axis = 1;
	}
	else if (size.z > size.x && size.z > size.y)
	{
		axis = 2;
	}

	float split_position{ center.x };

	if (axis == 1)
	{
		split_position = center.y;
	}
	else if (axis == 2)
	{
		split_position = center.z;
	}

	uint32_t left{ begin };
	uint32_t right{ end - 1 };

	while (left <= right)
	{
		float value = 0.0f;

		if (axis == 0)
		{
			value = centroids[bvh_tri_indices[left]].x;
		}
		else if (axis == 1)
		{
			value = centroids[bvh_tri_indices[left]].y;
		}
		else
		{
			value = centroids[bvh_tri_indices[left]].z;
		}

		if (value < split_position)
		{
			++left;
		}
		else
		{
			const uint32_t temp{ bvh_tri_indices[left] };
			bvh_tri_indices[left] = bvh_tri_indices[right];
			bvh_tri_indices[right] = temp;

			if (right == 0)
			{
				break;
			}

			--right;
		}
	}

	uint32_t mid{ left };

	if (mid <= begin || mid >= end)
	{
		mid = begin + count / 2;
	}

	const uint32_t left_child{ BuildBVHNode(begin, mid, aabbs, centroids) };


	const uint32_t right_child{ BuildBVHNode(mid, end, aabbs, centroids) };

	bvh_nodes[node_idx].left_child = left_child;

	// Store the right child in first_tri for internal nodes.
	bvh_nodes[node_idx].first_tri = right_child;

	return node_idx;
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