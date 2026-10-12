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
	const uint32_t triangle_count{ static_cast<uint32_t>(indices.size() / 3) };
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

		const uint32_t aabb_idx{ static_cast<uint32_t>(aabbs.size()) };
		aabbs.push_back(AABB{});


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
		bvh_nodes.emplace_back(); // Create root at index 0.

		BuildBVHNode(bvh_nodes, bvh_tri_indices, aabbs, centroids, 0, 0, triangle_count);
	}
}

//Credits to Robbe
void gfx::BuildBVHNode(
	std::vector<BVHNode>& bvh_nodes,
	std::vector<uint32_t>& bvh_tri_indices,
	const std::vector<AABB>& triangle_aabbs,
	const std::vector<Vector3>& triangle_centroids,
	const uint32_t node_index, const uint32_t start, const uint32_t end)
{
	enum class SplitAxis
	{
		kNone, kX, kY, kZ
	};

	BVHNode& node{ bvh_nodes[node_index] };

	// 4. Create and bound the node.
	for (uint32_t i{ start }; i < end; ++i)
	{
		const uint32_t triangle_index{ bvh_tri_indices[i] };
		node.bounds.ExpandToInclude(triangle_aabbs[triangle_index]);
	}

	const uint32_t triangle_count{ end - start };

	// 5. Subdivide nodes until a leaf condition is met.
	constexpr uint32_t kMaxTrianglesPerLeaf{ 4 };

	if (triangle_count <= kMaxTrianglesPerLeaf)
	{
		node.first_tri = start;
		node.tri_count = triangle_count;
		return;
	}

	// 6. Choose a split axis and position.
	AABB centroid_bounds{};

	for (uint32_t i{ start }; i < end; ++i)
	{
		const uint32_t triangle_index{ bvh_tri_indices[i] };
		centroid_bounds.ExpandToInclude(
			triangle_centroids[triangle_index]
		);
	}

	const Vector3 centroid_size{
		centroid_bounds.GetSize()
	};

	SplitAxis split_axis{ SplitAxis::kNone };
	float split_position{};
	// X
	if (centroid_size.x > centroid_size.y && centroid_size.x > centroid_size.z)
	{
		split_axis = SplitAxis::kX;
		split_position =
			(centroid_bounds.min.x + centroid_bounds.max.x) * 0.5f;
	}
	// Y
	else if (centroid_size.y > centroid_size.x && centroid_size.y > centroid_size.z)
	{
		split_axis = SplitAxis::kY;
		split_position =
			(centroid_bounds.min.y + centroid_bounds.max.y) * 0.5f;
	}
	// Z
	else
	{
		split_axis = SplitAxis::kZ;
		split_position =
			(centroid_bounds.min.z + centroid_bounds.max.z) * 0.5f;
	}
	// 7. Partition the node's indices around the split.
	//      Handle a one - sided partition!

	// Order first by position < split axis then position > split axis
	// "std::partition rearranges elements and returns an iterator pointing to the first element of the second group (the elements that evaluated to false)."
	// "std::distance or pointer subtraction (it - v.begin()) calculates the integer index of that boundary in O(1) time."
	auto middle{
		std::partition(bvh_tri_indices.begin() + start, bvh_tri_indices.begin() + end,[&](const uint32_t triangle_index)
			{
				const Vector3& centroid { triangle_centroids[triangle_index] };

				float value{};
				if (split_axis == SplitAxis::kX)
				{
					value = centroid.x;
				}
				else if (split_axis == SplitAxis::kY)
				{
					value = centroid.y;
				}
				else if (split_axis == SplitAxis::kZ)
				{
					value = centroid.z;
				}

				return value < split_position;
			}
		)
	};
	const uint32_t middle_index{ static_cast<uint32_t>(std::distance(bvh_tri_indices.begin(), middle)) };

	// No usefull split 
	if (middle_index == start || middle_index == end)
	{
		node.first_tri = start;
		node.tri_count = triangle_count;
		return;
	}
	// 8. Create and bound two child nodes if split happened.
	//      Here you can use the helper function from before.

	// set left_child_index after the last node 
	const uint32_t left_child_index{ static_cast<uint32_t>(bvh_nodes.size()) };

	// create two nodes => having left_child_index pointing to the first one
	bvh_nodes.emplace_back();
	bvh_nodes.emplace_back();
	// 9. Mark the node internal and recurse into it children.
	node.left_child = left_child_index;

	// Left node
	BuildBVHNode(
		bvh_nodes,
		bvh_tri_indices,
		triangle_aabbs,
		triangle_centroids,
		left_child_index,
		start,
		middle_index
	);
	// Right node
	BuildBVHNode(
		bvh_nodes,
		bvh_tri_indices,
		triangle_aabbs,
		triangle_centroids,
		left_child_index + 1,
		middle_index,
		end
	);
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