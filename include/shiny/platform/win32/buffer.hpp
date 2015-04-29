#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>
#include <atma/vector.hpp>


namespace shiny
{
	namespace detail
	{
		template <typename T>
		using typed_shadow_buffer_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;

		using shadow_buffer_t = typed_shadow_buffer_t<byte>;




		struct gen_view_t
		{
			gen_view_t(bool g, gpu_access_t ga, element_format_t ef, resource_subset_t rs)
				: generate(g), gpu_access(ga), element_format(ef), subset(rs)
			{}

			bool generate;
			gpu_access_t gpu_access;
			element_format_t element_format;
			resource_subset_t subset;
		};

		template <typename T>
		struct buffer_data_vtable_t;


		// raw-pointer
		template <>
		struct buffer_data_vtable_t<void const*>
		{
			static auto shadowbuffer_copy(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto ptr = reinterpret_cast<byte const*>(srcptr);
				dest.assign(ptr, ptr + size);
			}

			static auto shadowbuffer_move(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto ptr = const_cast<void*>(srcptr);
				auto t = shadow_buffer_t::buffer_type{atma::unique_memory_take_ownership_tag{}, ptr, size};
				dest.attach_buffer(std::move(t));
			}

			static auto vram_upload(void const*& dest, void const* srcptr, size_t size) -> void
			{
				dest = srcptr;
			}

			static auto vram_postupload_move(void const* srcptr, size_t) -> void
			{
				delete srcptr;
			}

			static auto vram_postupload_copy(void const*, size_t) -> void
			{}
		};

		// typed-shadow-buffer
		template <typename T>
		struct buffer_data_vtable_t<typed_shadow_buffer_t<T>>
		{
			static auto shadowbuffer_copy(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto const& src = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				dest.assign(src.begin(), src.end());
			}

			static auto shadowbuffer_move(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				auto& src = const_cast<typed_shadow_buffer_t<T>&>(csrc);
				dest.attach_buffer(src.detach_buffer());
			}

			static auto vram_upload(void const*& dest, void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				dest = csrc.data();
			}

			static auto vram_postupload_move(void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				auto& src = const_cast<typed_shadow_buffer_t<T>&>(csrc);
				src.detach_buffer();
			}
		};
	}

	struct gen_default_read_view_t : detail::gen_view_t
	{
		gen_default_read_view_t(element_format_t ef = element_format_t::unknown, resource_subset_t rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read, ef, rs}
		{}
	};

	struct gen_default_read_write_view_t : detail::gen_view_t
	{
		gen_default_read_write_view_t(element_format_t ef = element_format_t::unknown, resource_subset_t const& rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read_write, ef, rs}
		{}
	};


	struct buffer_dimensions_t
	{
		buffer_dimensions_t(size_t stride, size_t count)
			: stride(stride), count(count)
		{
		}

		size_t stride;
		size_t count;

		template <typename T>
		static auto infer(detail::typed_shadow_buffer_t<T> const& memory) -> buffer_dimensions_t
		{
			return buffer_dimensions_t{sizeof(T), memory.size()};
		}
	};

	struct buffer_data_t
	{
		buffer_data_t(buffer_data_t const&) = delete;
		buffer_data_t(buffer_data_t&&) = delete;

		static auto copy(void const* data, size_t size) -> buffer_data_t
		{
			using vtable = detail::buffer_data_vtable_t<void const*>;

			return buffer_data_t{data, size,
				&vtable::vram_upload,
				&vtable::vram_postupload_copy,
				&vtable::shadowbuffer_copy};
		}

		template <typename T>
		static auto copy(detail::typed_shadow_buffer_t<T>& buf) -> buffer_data_t
		{
			using vtable = detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>;

			return buffer_data_t{&buf, buf.size(),
				&vtable::vram_upload,
				&vtable::vram_postupload_copy,
				&vtable::shadowbuffer_copy};
		}

		template <typename T>
		static auto move(detail::typed_shadow_buffer_t<T>& buf) -> buffer_data_t
		{
			using vtable = detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>;

			return buffer_data_t{&buf, buf.size(),
				&vtable::vram_upload,
				&vtable::vram_postupload_move,
				&vtable::shadowbuffer_move};
		}

	private:
		auto apply_to_vram(void const*& dest) const -> void
		{
			vram_fn_(dest, data, count);
		}

		auto post_vram() const -> void
		{
			vram_post_fn_(data, count);
		}

		auto apply_to_shadowbuffer(detail::shadow_buffer_t& buf) const -> void
		{
			shadowbuffer_fn_(buf, data, count);
		}

	private:
		using vram_fnptr         = void(*)(void const*&, void const*, size_t);
		using vram_post_fnptr    = void(*)(void const*, size_t);
		using shadowbuffer_fnptr = void(*)(detail::shadow_buffer_t&, void const*, size_t);

		buffer_data_t(void const* data, size_t count, vram_fnptr vf, vram_post_fnptr vpf, shadowbuffer_fnptr sf)
			: data(data), count(count)
			, vram_fn_(vf), vram_post_fn_(vpf), shadowbuffer_fn_(sf)
		{}

		void const* data;
		size_t count;
		vram_fnptr         vram_fn_;
		vram_post_fnptr    vram_post_fn_;
		shadowbuffer_fnptr shadowbuffer_fn_;

		friend struct buffer_t;
	};

	struct buffer_t : resource_t
	{
		//template <typename T>
		//using typed_shadow_buffer_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;
		//using shadow_buffer_t = typed_shadow_buffer_t<char>;
		template <typename T>
		using typed_shadow_buffer_t = detail::typed_shadow_buffer_t<T>;
		using shadow_buffer_t = detail::shadow_buffer_t;


		buffer_t(context_ptr const&, resource_type_t, resource_usage_mask_t, resource_storage_t, buffer_dimensions_t const&, buffer_data_t const&);
		virtual ~buffer_t();

		auto buffer_usage() const -> resource_storage_t { return buffer_usage_; }
		auto is_shadowing() const -> bool { return !shadow_buffer_.empty(); }
		
		auto default_read_view() const -> resource_view_ptr const& { return default_read_view_; }
		auto default_read_write_view() const -> resource_view_ptr const& { return default_read_write_view_; }

		auto bind(gen_default_read_view_t const&) -> void;
		auto bind(gen_default_read_write_view_t const&) -> void;

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }
		auto d3d_resource() const -> platform::d3d_resource_ptr override { return d3d_buffer_; }

	protected:
		auto upload_shadow_buffer() -> void;

	protected:
		resource_storage_t buffer_usage_;

		shadow_buffer_t shadow_buffer_;

		resource_view_ptr default_read_view_;
		resource_view_ptr default_read_write_view_;

		platform::d3d_buffer_ptr d3d_buffer_;
	};




	template <typename... Args>
	inline auto make_buffer(context_ptr const& ctx,
		resource_type_t rt,
		resource_usage_mask_t ru,
		resource_storage_t bu,
		buffer_dimensions_t const& bdm,
		buffer_data_t const& bdt,
		Args&&... req_views) -> buffer_ptr
	{
		auto b = atma::make_intrusive_ptr<buffer_t>(ctx, rt, ru, bu, bdm, bdt);
		int _[] = { 0, (b->bind(req_views), 0)... };
		
		return b;
	}

}
