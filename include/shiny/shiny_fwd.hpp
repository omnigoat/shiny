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
	using  constant_buffer_ptr  = atma::intrusive_ptr<constant_buffer_t>;
	using  constant_buffer_cptr = atma::intrusive_ptr<constant_buffer_t const>;
	using  bound_constant_buffer_t  = std::pair<uint, constant_buffer_cptr>;
	using  bound_constant_buffers_t = std::vector<bound_constant_buffer_t>;

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
	using  compute_shader_ptr  = atma::intrusive_ptr<compute_shader_t>;
	using  compute_shader_cptr = atma::intrusive_ptr<compute_shader_t const>;

	struct resource_t;
	using  resource_ptr  = atma::intrusive_ptr<resource_t>;
	using  resource_cptr = atma::intrusive_ptr<resource_t const>;
	using  bound_resource_t  = std::pair<uint, resource_cptr>;
	using  bound_resources_t = std::vector<bound_resource_t>;

	struct resource_view_t;
	using  resource_view_ptr  = atma::intrusive_ptr<resource_view_t>;
	using  resource_view_cptr = atma::intrusive_ptr<resource_view_t const>;

	struct bound_resource_view_t
	{
		bound_resource_view_t(uint, resource_view_cptr const&);
		bound_resource_view_t(uint, resource_view_cptr const&, uint);

		uint idx;
		resource_view_cptr view;
		uint counter; // ~0 == do nothing
	};

	using  bound_resource_views_t = std::vector<bound_resource_view_t>;

	struct buffer_t;
	using  buffer_ptr  = atma::intrusive_ptr<buffer_t>;
	using  buffer_cptr = atma::intrusive_ptr<buffer_t>;

	struct generic_buffer_t;
	typedef atma::intrusive_ptr<generic_buffer_t> generic_buffer_ptr;
	typedef atma::intrusive_ptr<generic_buffer_t const> generic_buffer_cptr;

	struct structured_buffer_t;
	using  structured_buffer_ptr  = atma::intrusive_ptr<structured_buffer_t>;
	using  structured_buffer_cptr = atma::intrusive_ptr<structured_buffer_t const>;

	struct blender_t;
	using  blender_ptr  = atma::intrusive_ptr<blender_t>;
	using  blender_cptr = atma::intrusive_ptr<blender_t const>;

	struct draw_target_t;
	using  draw_target_ptr = atma::intrusive_ptr<draw_targe_t>;

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
		staging_buffer,

		texture2d,
		texturd3d,
	};

	enum class resource_view_type_t
	{
		// read-only (shader-resource)
		input,
		// read-write (unordered-access)
		compute,

		render_target,
		depth_stencil,
	};




	enum class resource_storage_t
	{
		// immutable:
		//   cpu: none
		//   gpu: read
		immutable,

		// persistant:
		//   cpu: write
		//   gpu: read (write with unordered-access)
		//   update: update-subresource (d3d)
		//   NOTE: can't update mid-frame. need to be updated in the 
		//         resource-upload stage
		persistant,

		// transient:
		//   cpu: write
		//   gpu: read
		//   update: map/discard
		//   NOTE: these should be pooled
		transient,

		// temporary:
		//   cpu: write
		//   gpu: read
		//   update: map/unmap
		//   NOTE: can update mid-frame
		temporary,

		// constant:
		//   cpu: write
		//   gpu: read
		//   update: map/discard
		constant,

		// staging:
		//   cpu: read
		//   gpu: write
		staging,
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


