#pragma once
#include <atma/intrusive_ptr.hpp>

#include <shiny/shiny_fwd.hpp>

#include <typeindex>


namespace shiny
{
	struct resource_t : atma::ref_counted
	{
		resource_t(renderer_ptr const& rndr, resource_type_t rt, resource_usage_mask_t ru, resource_storage_t rs, size_t element_stride, size_t element_count)
			: rndr_{rndr}
			, resource_type_{rt}
			, resource_usage_{ru}
			, resource_storage_{rs}
			, element_stride_{element_stride}
			, element_count_{element_count}
		{}

		virtual ~resource_t()
		{}

		virtual auto sizeof_host_resource() const -> size_t = 0;

		auto renderer() const -> renderer_ptr const& { return rndr_; }
		auto resource_type() const -> resource_type_t { return resource_type_; }
		auto resource_usage() const -> resource_usage_mask_t { return resource_usage_; }
		auto resource_storage() const -> resource_storage_t { return resource_storage_; }
		auto elements_stride() const -> size_t { return element_stride_; }
		auto elements_count() const -> size_t { return element_count_; }
		auto resource_size() const -> size_t { return element_stride_ * element_count_; }

	private:
		renderer_ptr rndr_;
		resource_type_t resource_type_ = resource_type_t::unknown;
		resource_usage_mask_t resource_usage_;
		resource_storage_t resource_storage_ = resource_storage_t::unknown;
		size_t element_stride_ = 0;
		size_t element_count_ = 0;

		template <typename R, typename T> friend struct device_pin_ptr;
	};
}

namespace shiny
{
	template <typename R, typename T>
	struct device_pin_ptr
	{
		using devce_type = atma::transfer_const_t<typename R::value_type, T>;

		device_pin_ptr()
		{}

		explicit device_pin_ptr(R const& r)
			: r_{r}
		{}

		device_pin_ptr(device_pin_ptr const& rhs)
			: r_{rhs.r_}
		{}

		device_pin_ptr(device_pin_ptr&& rhs)
			: r_{std::move(rhs.r_)}
		{}

		auto operator = (device_pin_ptr<R,T> const& rhs) -> device_pin_ptr<R,T>&
		{
			r_ = rhs.r_;
			return *this;
		}

		auto operator = (device_pin_ptr<R,T>&& rhs) -> device_pin_ptr<R,T>&
		{
			r_ = std::move(rhs.r_);
			return *this;
		}

		operator bool () const {
			return r_;
		}

		auto operator ! () const -> bool {
			return !r_;
		}

		auto operator -> () const -> devce_type*
		{
			using intermediary_t = atma::transfer_const_t<typename R::value_type, char>;
			return r_ ? reinterpret_cast<devce_type*>(reinterpret_cast<intermediary_t*>(r_.get()) + r_->sizeof_host_resource()) : nullptr;
		}

		auto operator * () const -> T&
		{
			ATMA_ASSERT(r_, "dereferencing nullptr");
			return *operator->();
		}

		static auto unsafe_access(R const& r)
		{
			using intermediary_t = atma::transfer_const_t<typename R::value_type, char>;
			return r ? reinterpret_cast<devce_type*>(reinterpret_cast<intermediary_t*>(r.get()) + r->sizeof_host_resource()) : nullptr;
		}

	private:
		R r_;
	};


	template <typename T>
	inline auto device_pin(resource_ptr const& r)
	{
		return device_pin_ptr<resource_ptr, T>{r};
	}

	template <typename T>
	inline auto device_pin(resource_cptr const& r)
	{
		return device_pin_ptr<resource_cptr, T>{r};
	}

	template <typename T, typename R>
	inline auto device_unsafe_access(R const& r)
	{
		return device_pin_ptr<R, T>::unsafe_access(r);
	}
}

namespace shiny
{
#if 0
	template <typename C, typename I>
	inline constexpr C* inteface_to_concrete(I* i)
	{
		return reinterpret_cast<C*>((char*)i + sizeof(I));
	}

	template <typename I, typename C>
	inline constexpr C* concrete_to_interface(C* c)
	{
		return reinterpret_cast<I*>((char*)c - sizeof(I));
	}
#endif

	template <typename Interface, typename Concrete>
	struct resource_bridge_t
		: Interface
	{
		resource_bridge_t(Interface&& i, Concrete&& c)
			: Interface{std::move(i)}
			, concrete_{std::move(c)}
		{}

		// helpful if host & device constructors are identical
		template <typename... Args>
		resource_bridge_t(Args&&... args)
			: Interface{std::forward<Args>(args)...}
			, concrete_{std::forward<Args>(args)...}
		{}

	protected:
		Concrete concrete_;
	};
}
