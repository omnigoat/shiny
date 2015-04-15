#pragma once

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>

#include <vector>


namespace shiny
{
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

	struct resource_view_t;
	using  resource_view_ptr  = atma::intrusive_ptr<resource_view_t>;
	using  resource_view_cptr = atma::intrusive_ptr<resource_view_t const>;

	struct resource_t;
	//typedef atma::intrusive_ptr<resource_t> resource_ptr;
	using resource_ptr  = atma::intrusive_ptr<resource_t>;
	using resource_cptr = atma::intrusive_ptr<resource_t const>;
	typedef std::vector<std::pair<uint, resource_ptr>> bound_resources_t;

	struct generic_buffer_t;
	typedef atma::intrusive_ptr<generic_buffer_t> generic_buffer_ptr;
	typedef atma::intrusive_ptr<generic_buffer_t const> generic_buffer_cptr;

	struct structured_buffer_t;
	using  structured_buffer_ptr  = atma::intrusive_ptr<structured_buffer_t>;
	using  structured_buffer_cptr = atma::intrusive_ptr<structured_buffer_t const>;

	struct blender_t;
	using  blender_ptr  = atma::intrusive_ptr<blender_t>;
	using  blender_cptr = atma::intrusive_ptr<blender_t const>;

	struct mapped_subresource_t
	{
		void* data;
		uint pitch_y, pitch_z;
	};


	enum class resource_type_t
	{
		vertex_buffer,
		index_buffer,
		constant_buffer,
		generic_buffer,
		structured_buffer,

		texture2d,
		texturd3d,
	};





	enum class buffer_usage_t
	{
		// immutable:
		//   cpu: none
		//   gpu: read
		immutable,

		// persistant:
		//   cpu: write
		//   gpu: read (write with unordered-access)
		//   update: update-subresource (d3d)
		persistant,
		persistant_shadowed,

		// temporary:
		//   cpu: write
		//   gpu: read
		//   update: map/unmap
		temporary,
		temporary_shadowed,

		// transient:
		//   cpu: write
		//   gpu: read
		//   update: map/discard
		//   NOTE: these should be pooled
		transient,
		transient_shadowed,

		// constant:
		//   cpu: write
		//   gpu: read
		//   update: map/discard
		constant,
		constant_shadowed,
	};



	enum class pipeline_stage_t
	{
		input_assembly,
		vertex,
		geometry,
		fragment,
	};

	ATMA_BITMASK(pipeline_stage_mask_t, pipeline_stage_t);

	enum class resource_usage_t
	{
		render_target,
		depth_stencil,
		shader_resource,
		unordered_access,
	};

	ATMA_BITMASK(resource_usage_mask_t, resource_usage_t);

	enum class gpu_access_t
	{
		none,
		read,
		write,
		read_write
	};

	enum class view_type_t
	{
		buffer,
		texture2d,
		texture3d,
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

	enum class data_stream_stage_t
	{
		input_assembly,
		vertex,
	};

}


