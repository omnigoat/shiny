#pragma once
//======================================================================
#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>

#include <vector>
//======================================================================
namespace shiny {
	
	struct runtime_t;

	struct adapter_t;
	typedef atma::intrusive_ptr<adapter_t> adapter_ptr;

	struct output_t;
	typedef atma::intrusive_ptr<output_t> output_ptr;

	struct context_t;
	typedef atma::intrusive_ptr<context_t> context_ptr;

	struct geometry_shader_t;
	using  geometry_shader_ptr = atma::intrusive_ptr<geometry_shader_t>;
	using  geometry_shader_cptr = atma::intrusive_ptr<geometry_shader_t const>;

	struct vertex_shader_t;
	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;
	typedef atma::intrusive_ptr<vertex_shader_t const> vertex_shader_cptr;

	struct fragment_shader_t;
	typedef atma::intrusive_ptr<fragment_shader_t> fragment_shader_ptr;
	typedef atma::intrusive_ptr<fragment_shader_t const> fragment_shader_cptr;

	struct geometry_stream_t;
	using  geometry_streams_t = std::vector<geometry_stream_t>;
	struct geometry_declaration_t;

	struct data_stream_t;
	typedef std::vector<data_stream_t> data_streams_t;
	struct data_declaration_t;

	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;
	typedef atma::intrusive_ptr<vertex_buffer_t const> vertex_buffer_cptr;

	struct constant_buffer_t;
	typedef atma::intrusive_ptr<constant_buffer_t> constant_buffer_ptr;
	typedef atma::intrusive_ptr<constant_buffer_t const> constant_buffer_cptr;
	typedef std::vector<std::pair<uint, constant_buffer_cptr>> bound_constant_buffers_t;

	struct index_buffer_t;
	typedef atma::intrusive_ptr<index_buffer_t> index_buffer_ptr;
	typedef atma::intrusive_ptr<index_buffer_t const> index_buffer_cptr;

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
	typedef std::vector<std::pair<uint, resource_ptr>> bound_resources_t;

	struct generic_buffer_t;
	typedef atma::intrusive_ptr<generic_buffer_t> generic_buffer_ptr;

	struct blender_t;
	using  blender_ptr  = atma::intrusive_ptr<blender_t>;
	using  blender_cptr = atma::intrusive_ptr<blender_t const>;

	struct mapped_subresource_t
	{
		void* data;
		uint pitch_y, pitch_z;
	};


	enum class buffer_type_t
	{
		vertex_buffer,
		index_buffer,
		constant_buffer,
		generic_buffer,

		// maybe for unordered access views?
		//generic_random_buffer,
	};

	enum class buffer_usage_t
	{
		// gpu-only
		immutable,
		dynamic,
		long_lived,

		// backed by shadow-buffer
		dynamic_shadowed,
		long_lived_shadowed,
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

	enum class topology_t
	{
		point,
		line,
		triangle
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


