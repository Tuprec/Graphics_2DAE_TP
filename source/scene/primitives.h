//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef PRIMITIVES_HEADER
#define PRIMITIVES_HEADER

//--- Standard Includes ---
#include <cmath>
#include <limits>
#include <optional>
#include <vector>
#include <array>
#include <memory>

//--- Framework Includes ---
#include <vector2.h>
#include <vector3.h>
#include <color.h>

namespace gfx
{
	//--- Ray Tracing Support Structs ---
	struct Ray final
	{
		Vector3 origin{};
		Vector3 direction{};
		float min{ 1e-4f };
		float max{ std::numeric_limits<float>::max() };
	};

	struct RayHitRecord final
	{
		Ray ray{};
		float t{ std::numeric_limits<float>::max() };
		uint32_t object_index{ std::numeric_limits<uint32_t>::max() };
		std::optional<std::array<uint32_t, 3>> vertex_indices{};
		std::optional<Vector2> barycentric_coordinates{};
	};

	//--- Rasterization Support Structs ---
	struct RasterHitRecord final
	{
		uint32_t object_index{ std::numeric_limits<uint32_t>::max() };
		std::optional<std::array<uint32_t, 3>> vertex_indices{};
		std::optional<Vector2> barycentric_coordinates{};
		std::optional<Vector2> pixel_coordinate{};
	};

	//--- General Supporting Primitives ---
	struct AABB final
	{
		Vector3 min{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		Vector3 max{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

		AABB() = default;
		AABB(const Vector3& min_point, const Vector3& max_point) : min(min_point), max(max_point) {}

		void ExpandToInclude(const Vector3& point);
		void ExpandToInclude(const AABB& other);
		[[nodiscard]] bool Contains(const Vector3& point) const;
		[[nodiscard]] bool Overlaps(const AABB& other) const;
		[[nodiscard]] Vector3 GetCenter() const;
		[[nodiscard]] Vector3 GetSize() const;
		[[nodiscard]] bool IsValid() const;
	};

	struct BVHNode final
	{
		AABB bounds{};
		uint32_t left_child{ 0 };
		uint32_t first_tri{ 0 };
		uint32_t tri_count{ 0 };
		[[nodiscard]] bool IsLeaf() const { return tri_count > 0; }
	};

	//--- Base Primitive ---
	enum class PrimitiveType : uint8_t
	{
		kNone,
		kSphere,
		kPlane,
		kTriangle,
		kTriangleMesh
	};

	struct Primitive
	{
		PrimitiveType type{ PrimitiveType::kNone };
		std::optional<AABB> bounding_box{};

		virtual ~Primitive() = default;
		virtual Primitive* CloneIntoMemory(void* memory) const = 0;
		[[nodiscard]] virtual std::unique_ptr<Primitive> Clone() const = 0;
		template<typename T>
		[[nodiscard]] std::unique_ptr<T> CloneAs() const
		{
			return std::unique_ptr<T>(static_cast<T*>(Clone().release()));
		}
		[[nodiscard]] virtual uint64_t GetSize() const = 0;
		[[nodiscard]] virtual uint64_t GetAlignment() const = 0;
		virtual void UpdateAABB() = 0;

	protected:
		Primitive(const PrimitiveType t) : type(t) {}
		Primitive(const Primitive&) = default;
		Primitive& operator=(const Primitive&) = default;
		Primitive(Primitive&&) = default;
		Primitive& operator=(Primitive&&) = default;
	};

	//--- Ray Tracing Implicit Surfaces ---
	struct Sphere final : public Primitive
	{
		Vector3 origin{ 0.f, 0.f, 0.f };
		float radius{ 1.f };
		Sphere() : Primitive(PrimitiveType::kSphere) {}
		Sphere(const Vector3& o, const float r) :
			Primitive(PrimitiveType::kSphere),
			origin(o), radius(r)
		{
			UpdateAABB();
		}
		Sphere(const float r) :
			Primitive(PrimitiveType::kSphere),
			origin(), radius(r)
		{
			UpdateAABB();
		}

		Primitive* CloneIntoMemory(void* memory) const override
		{
			return new (memory) Sphere(*this);
		}
		[[nodiscard]] std::unique_ptr<Primitive> Clone() const override
		{
			return std::make_unique<Sphere>(*this);
		}
		[[nodiscard]] uint64_t GetSize() const override
		{
			return sizeof(Sphere);
		}
		[[nodiscard]] uint64_t GetAlignment() const override
		{
			return alignof(Sphere);
		}

		void UpdateAABB() override;
	};

	struct Plane final : public Primitive
	{
		Vector3 origin{ 0.f, 0.f, 0.f };
		Vector3 normal{ 0.f, 1.f, 0.f };
		Vector3 tangent{ 1.f, 0.f, 0.f };
		bool double_sided{ true };
		std::optional<Vector2> half_extent{};
		Plane() : Primitive(PrimitiveType::kPlane) { BuildTangent(); }
		Plane(const Vector3& o, const Vector3& n, const bool is_double_sided = true, const std::optional<Vector2>& extent = {}) :
			Primitive(PrimitiveType::kPlane),
			origin(o), normal(n), double_sided(is_double_sided), half_extent(extent)
		{
			BuildTangent();
		}
		Plane(const Vector3& n, const bool is_double_sided = true) :
			Primitive(PrimitiveType::kPlane),
			origin(), normal(n), double_sided(is_double_sided)
		{
			BuildTangent();
		}

		Primitive* CloneIntoMemory(void* memory) const override
		{
			return new (memory) Plane(*this);
		}
		[[nodiscard]] std::unique_ptr<Primitive> Clone() const override
		{
			return std::make_unique<Plane>(*this);
		}
		[[nodiscard]] uint64_t GetSize() const override
		{
			return sizeof(Plane);
		}
		[[nodiscard]] uint64_t GetAlignment() const override
		{
			return alignof(Plane);
		}

		void UpdateAABB() override {}

	private:
		// Building an Orthonormal Basis (Duff et al., JCGT 2017).
		// https://jcgt.org/published/0006/01/01/
		// Builds a tangent from the normal for any orientation.
		void BuildTangent()
		{
			const float sign = std::copysign(1.0f, normal.z);
			const float a = -1.0f / (sign + normal.z);
			const float b = normal.x * normal.y * a;
			tangent = Vector3(1.0f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
		}
	};

	//--- Lights ---
	enum class LightType : uint8_t
	{
		kNone,
		kPoint,
		kDirectional
	};

	struct Light final
	{
		Vector3 origin{ 0.f, 0.f, 0.f };
		Vector3 direction{ 0.f, 0.f, 0.f };
		ColorRgba color{ ColorRgba::White() };
		float intensity{ 1.f };
		LightType type{ LightType::kNone };

		// Required by Factory<Light>::Clone()
		Light* CloneIntoMemory(void* memory) const { return new (memory) Light(*this); }
		[[nodiscard]] std::unique_ptr<Light> Clone() const { return std::make_unique<Light>(*this); }
		[[nodiscard]] uint64_t GetSize() const { return sizeof(Light); }
		[[nodiscard]] uint64_t GetAlignment() const { return alignof(Light); }
	};

	//--- Explicit Surfaces ---
	enum class CullMode : uint8_t
	{
		kBackFaceCulling,
		kFrontFaceCulling,
		kNoCulling
	};

	// Do not use ColorRgba for color as alpha is not supported!
	struct Vertex final
	{
		Vector4 position{ 0.f, 0.f, 0.f, 1.f };
		std::optional<Vector3> color{};
		std::optional<Vector3> normal{};
		std::optional<Vector3> tangent{};
		std::optional<Vector3> bitangent{};
		std::optional<Vector2> texture_coordinates{};
	};

	// Do not use triangle with mesh! Vertex struct defined above should be used in mesh!
	struct Triangle final : public Primitive
	{
		Vector3 v0{ 0.f, 0.f, 0.f };
		Vector3 v1{ 0.f, 0.f, 0.f };
		Vector3 v2{ 0.f, 0.f, 0.f };
		Vector3 normal{ 0.f, 0.f, 0.f };
		CullMode cull_mode{};

		Triangle() : Primitive(PrimitiveType::kTriangle) {}
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal,
			const CullMode mode = CullMode::kBackFaceCulling) : Primitive(PrimitiveType::kTriangle),
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }, normal{ _normal.Normalized() }, cull_mode(mode)
		{
			UpdateAABB();
		}
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2,
			const CullMode mode = CullMode::kBackFaceCulling) : Primitive(PrimitiveType::kTriangle),
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }, cull_mode(mode)
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
			UpdateAABB();
		}

		Primitive* CloneIntoMemory(void* memory) const override
		{
			return new (memory) Triangle(*this);
		}
		[[nodiscard]] std::unique_ptr<Primitive> Clone() const override
		{
			return std::make_unique<Triangle>(*this);
		}
		[[nodiscard]] uint64_t GetSize() const override
		{
			return sizeof(Triangle);
		}
		[[nodiscard]] uint64_t GetAlignment() const override
		{
			return alignof(Triangle);
		}

		void UpdateAABB() override;
	};

	enum class PrimitiveTopology : uint8_t
	{
		kTriangleList,
		kTriangleStrip
	};

	struct TriangleMesh final : public Primitive
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		CullMode cull_mode{ CullMode::kBackFaceCulling };
		PrimitiveTopology primitive_topology{ PrimitiveTopology::kTriangleList };

		// BVH acceleration structure (built during UpdateAABB)
		std::vector<BVHNode> bvh_nodes{};
		std::vector<uint32_t> bvh_tri_indices{}; // Reordered triangle indices into the BVH

		TriangleMesh() : Primitive(PrimitiveType::kTriangleMesh) {}
		TriangleMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
			const CullMode mode = CullMode::kBackFaceCulling,
			const PrimitiveTopology topology = PrimitiveTopology::kTriangleList)
			: Primitive(PrimitiveType::kTriangleMesh),
			vertices(vertices), indices(indices),
			cull_mode(mode), primitive_topology(topology)
		{
			UpdateAABB();
		}
		TriangleMesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices,
			const CullMode mode = CullMode::kBackFaceCulling,
			const PrimitiveTopology topology = PrimitiveTopology::kTriangleList)
			: Primitive(PrimitiveType::kTriangleMesh),
			vertices(std::move(vertices)), indices(std::move(indices)),
			cull_mode(mode), primitive_topology(topology)
		{
			UpdateAABB();
		}

		Primitive* CloneIntoMemory(void* memory) const override
		{
			return new (memory) TriangleMesh(*this);
		}
		[[nodiscard]] std::unique_ptr<Primitive> Clone() const override
		{
			return std::make_unique<TriangleMesh>(*this);
		}
		[[nodiscard]] uint64_t GetSize() const override
		{
			return sizeof(TriangleMesh);
		}
		[[nodiscard]] uint64_t GetAlignment() const override
		{
			return alignof(TriangleMesh);
		}

		void UpdateAABB() override;
	};
}

#endif //PRIMITIVES_HEADER