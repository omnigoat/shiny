#pragma once
//======================================================================
#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust {
	
	struct runtime_t;

	struct adapter_t;
	typedef atma::intrusive_ptr<adapter_t> adapter_ptr;

	struct output_t;
	typedef atma::intrusive_ptr<output_t> output_ptr;

	struct context_t;
	typedef atma::intrusive_ptr<context_t> context_ptr;

	struct vertex_shader_t;
	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;

	struct pixel_shader_t;
	typedef atma::intrusive_ptr<pixel_shader_t> pixel_shader_ptr;

	struct vertex_declaration_t;

	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;

	struct constant_buffer_t;
	typedef atma::intrusive_ptr<constant_buffer_t> constant_buffer_ptr;
	typedef atma::intrusive_ptr<constant_buffer_t const> constant_buffer_cptr;

	struct index_buffer_t;
	typedef atma::intrusive_ptr<index_buffer_t> index_buffer_ptr;

	struct camera_t;
	struct scene_t;

	struct texture2d_t;
	typedef atma::intrusive_ptr<texture2d_t> texture2d_ptr;

	struct texture3d_t;
	typedef atma::intrusive_ptr<texture3d_t> texture3d_ptr;

	struct compute_shader_t;
	typedef atma::intrusive_ptr<compute_shader_t> compute_shader_ptr;

	struct buffer_t;
	typedef atma::intrusive_ptr<buffer_t> buffer_ptr;

	struct shader_resource2d_t;
	typedef atma::intrusive_ptr<shader_resource2d_t> shader_resource2d_ptr;

	struct resource_t;
	typedef atma::intrusive_ptr<resource_t> resource_ptr;

	struct mapped_subresource_t
	{
		void* data;
		uint pitch_y, pitch_z;
	};



	enum class buffer_usage_t
	{
		immutable,
		long_lived,
		dynamic,
	};

	enum class buffer_type_t
	{
		vertex_buffer,
		index_buffer,
		constant_buffer,
	};

	enum class resource_usage_t
	{
		render_target,
		depth_stencil,
		shader_resource,
		unordered_access,
	};

	enum class view_type_t
	{
		read_only,
		read_write
	};

	enum class map_type_t
	{
		write,
		read,
		write_discard,
	};

	enum class texture_usage_t
	{
		render_target,
		depth_stencil,

		immutable,
		streaming,
	};

	template <typename T>
	struct bitflags_t
	{
		bitflags_t()
		: flags_()
		{}

		bitflags_t(T x)
		: flags_(1 << (uint)x)
		{}

		bitflags_t(std::initializer_list<T> const& xs)
		: flags_()
		{
			for (auto x : xs)
				flags_ |= 1 << (uint)x;
		}

		auto operator & (T x) const -> bool {
			return (flags_ & (1 << (uint) x)) != 0;
		}

		auto operator |= (T x) -> void {
			flags_ |= (1 << (uint)x);
		}

	private:
		uint32 flags_;
	};

	typedef bitflags_t<resource_usage_t> resource_usage_flags_t;

}


