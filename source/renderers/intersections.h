//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef INTERSECTIONS_HEADER
#define INTERSECTIONS_HEADER

//--- Standard Includes ---
#include <cassert>

//--- Framework Includes ---
#include <primitives.h>
#include <matrix.h>

namespace gfx
{
	//--- Intersection Tests ---
	[[maybe_unused]]
	static bool HitTestSphere(const Sphere& sphere, const Ray& ray, RayHitRecord& hit_record, const bool ignore_hit_record = false)
	{
		//Calculating a hit an a sphere using the geometric way

		const Vector3 l{ sphere.origin - ray.origin };
		const float tca{ Vector3::Dot(l,ray.direction) };
		const float l_dot{ Vector3::Dot(l, l) };
		const float d2{ l_dot - tca * tca };
		//early return to save computation power
		if (d2 > sphere.radius * sphere.radius)
		{
			return false;
		}
		//const float od{ sqrtf(d2) }; //==> not using the reject function gave a diviation of 2-4 pixel
		const float od{ powf(Vector3::Reject(l, ray.direction).Magnitude(), 2) };
		const float r2{ sphere.radius * sphere.radius };
		//early return to save computation power

		if (od > r2)
		{
			return false;
		}
		const float thc{ sqrtf(r2 - od) };
		const float t0{ tca - thc };
		//const float t1{ tca + thc }; // => Not needed because t0 wil always be the closest hit but here for completion

		//Check if t0 is withing the ray bounds
		if (t0 < ray.min || t0 > ray.max)
		{
			return false;
		}

		if (!ignore_hit_record)
		{
			//Check if the new t is not negatif(behind me) and if the new t is smaller than te old t(closer) 
			//if not return false thus "no hit" occured because the object is behind something and i don't care about what i don't see
			if (t0 >= 0.0f && t0 < hit_record.t)
			{
				hit_record.ray = ray;
				hit_record.t = t0;
			}
			else
			{
				return false;
			}
		}
		return t0 >= 0.0f;
	}

	[[maybe_unused]]
	static bool HitTestPlane(const Plane& plane, const Ray& ray, RayHitRecord& hit_record, const bool ignore_hit_record = false)
	{
		const float denominator = Vector3::Dot(ray.direction, plane.normal);
		//Check for 0 devision
		if (denominator == 0)
			return false;

		const float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / denominator;

		//Check if t is withing the ray bounds
		if (t < ray.min || t > ray.max)
		{
			return false;
		}

		//Check to see if plane is finite
		if (plane.half_extent.has_value())
		{
			const Vector2 bounds = plane.half_extent.value();
			const Vector3 bitangent = Vector3::Cross(plane.normal, plane.tangent);

			const Vector3 p = ray.origin + t * ray.direction;// => This is the hit point
			const Vector3 relative_p = p - plane.origin;//=> tis is the point p in the local space of the plane

			const float local_x = Vector3::Dot(relative_p, plane.tangent);
			const float local_y = Vector3::Dot(relative_p, bitangent);

			if (abs(local_x) > bounds.x || abs(local_y) > bounds.y)
			{
				return false;
			}
		}

		if (!ignore_hit_record)
		{
			//Check if the new t is not negatif(behind me) and if the new t is smaller than te old t(closer) 
			//if not return false thus "no hit" occured because the object is behind something and i don't care about what i don't see
			if (t < hit_record.t)
			{
				hit_record.ray = ray;
				hit_record.t = t;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	[[maybe_unused]]
	static bool HitTestTriangle(const Triangle& triangle, const Ray& ray,
		RayHitRecord& hit_record, const bool ignore_hit_record = false)
	{
		//TODO
		assert(false && "Not Implemented");
		(void)triangle; (void)ray; (void)hit_record; (void)ignore_hit_record;
		return false;
	}

	[[maybe_unused]]
	static bool HitTestAABB(const AABB& aabb, const Ray& ray)
	{
		//TODO
		assert(false && "Not Implemented");
		(void)aabb; (void)ray;
		return false;
	}
}
#endif //INTERSECTIONS_HEADER