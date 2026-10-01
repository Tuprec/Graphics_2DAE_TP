//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef FACTORY_HEADER
#define FACTORY_HEADER

//--- Standard Includes ---
#include <vector>
#include <memory>
#include <memory_resource>

namespace gfx
{
	constexpr uint64_t kKiloBytes{ 1024 };
	constexpr uint64_t kMegaBytes{ 1024 * kKiloBytes };
	constexpr uint64_t kGigaBytes{ 1024 * kMegaBytes };

	template<typename BaseType>
	class Factory final
	{
		//--- Data Members ---
		std::vector<std::byte> pool_buffer_{};
		std::pmr::monotonic_buffer_resource pool_{};
		std::vector<BaseType*> objects_{};
		uint64_t allocated_bytes_{ 0 };

	public:
		//--- Constructors & Destructor ---
		Factory(const uint64_t total_size_bytes = 2 * kMegaBytes) :
			pool_buffer_(total_size_bytes),
			pool_(pool_buffer_.data(), pool_buffer_.size())
		{}
		~Factory()
		{ Reset(); }
		Factory(const Factory&) = delete;
		Factory& operator=(const Factory&) = delete;
		Factory(Factory&&) noexcept = delete;
		Factory& operator=(Factory&&) noexcept = delete;

		//--- Functions ---
		template<typename T, typename... Args>
		uint32_t Create(Args&&... args)
		{
			static_assert(std::is_base_of_v<BaseType, T>, "T must derive from BaseType");
			const size_t size{ sizeof(T) };
			const size_t alignment{ alignof(T) };
			void* memory {pool_.allocate(size, alignment)};
			T* object = new (memory) T(std::forward<Args>(args)...);
			objects_.push_back(object);
			const size_t aligned_size{ (size + alignment - 1) & ~(alignment - 1) };
			allocated_bytes_ += aligned_size;
			return static_cast<uint32_t>(objects_.size() - 1);
		}

		[[nodiscard]] BaseType* Get(const uint32_t index) const
		{
			if (index >= static_cast<uint32_t>(objects_.size()))
				return nullptr;
			return objects_[index];
		}

		template<typename T>
		[[nodiscard]] T* GetAs(const uint32_t index) const
		{
			static_assert(std::is_base_of_v<BaseType, T>, "T must derive from BaseType");
			BaseType* object = Get(index);
			assert(!object || (dynamic_cast<T*>(object) && "GetAs: type mismatch"));
			return object ? static_cast<T*>(object) : nullptr;
		}

		[[nodiscard]] const std::vector<BaseType*>& GetAll() const { return objects_; }

		template<typename T>
		[[nodiscard]] std::vector<T*> GetAllOfType() const
		{
			static_assert(std::is_base_of_v<BaseType, T>, "T must derive from BaseType");
			std::vector<T*> result;
			for (BaseType* object : objects_)
			{
				if (T* casted = dynamic_cast<T*>(object))
					result.push_back(casted);
			}
			return result;
		}

		void Reset()
		{
			for (BaseType* object : objects_)
			{
				// Check if object is within our pool buffer
				const std::byte* obj_ptr { reinterpret_cast<const std::byte*>(object) };
				const std::byte* pool_start { pool_buffer_.data() };
				const std::byte* pool_end { pool_start + pool_buffer_.size() };

				// If object is in our pool, just call destructor. If it was
				// heap-allocated for some reason, delete it instead.
				if (obj_ptr >= pool_start && obj_ptr < pool_end)
					object->~BaseType();
				else
					delete object;
			}
			objects_.clear();
			pool_.release();
			allocated_bytes_ = 0;
		}

		[[nodiscard]] std::unique_ptr<Factory> Clone() const
		{
			auto cloned_factory = std::make_unique<Factory>(pool_buffer_.size());

			cloned_factory->objects_.reserve(objects_.size());
			for (const BaseType* original_object : objects_)
			{
				// Allocate memory in the cloned factory's pool using the object's actual size.
				const uint64_t object_size { original_object->GetSize() };
				const uint64_t object_alignment { original_object->GetAlignment() };
				const uint64_t object_aligned_size { (object_size + object_alignment - 1) & ~(object_alignment - 1) };
				void* memory { cloned_factory->pool_.allocate(object_size, object_alignment) };

				// Clone the object directly into the pool memory using placement new.
				BaseType* cloned_object = original_object->CloneIntoMemory(memory);

				cloned_factory->objects_.push_back(cloned_object);
				cloned_factory->allocated_bytes_ += object_aligned_size;
			}

			return cloned_factory;
		}
	};
}
#endif //FACTORY_HEADER