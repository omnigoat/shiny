#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/thread/engine.hpp>

namespace shiny
{
	struct shared_state_t
	{
		typedef std::vector<std::pair<uint, resource_ptr>> shader_resources_t;

		bound_resources_t shader_resources;
	};

	struct vertex_stage_state_t
	{
		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib, uint offset, uint count, bound_resources_t const& sr)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(offset), count(count), shader_resources(sr)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib, uint offset, uint count)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(offset), count(count)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(), count()
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, uint offset, uint count)
			: vertex_shader(vs), vertex_buffer(vb), offset(offset), count(count)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb)
			: vertex_shader(vs), vertex_buffer(vb), offset(), count()
		{}

		vertex_shader_cptr vertex_shader;
		vertex_buffer_cptr vertex_buffer;
		index_buffer_cptr index_buffer;
		bound_resources_t shader_resources;

		uint offset, count;
	};

	struct fragment_stage_state_t
	{
		fragment_stage_state_t(fragment_shader_cptr const& fs, bound_constant_buffers_t const& bcb, bound_resources_t const& bs)
			: fragment_shader(fs), constant_buffers(bcb), shader_resources(bs)
		{}

		fragment_stage_state_t(fragment_shader_cptr const& fs, bound_constant_buffers_t const& bcb)
			: fragment_shader(fs), constant_buffers(bcb)
		{}

		fragment_stage_state_t(fragment_shader_cptr const& fs)
			: fragment_shader(fs)
		{}

		fragment_shader_cptr fragment_shader;
		bound_constant_buffers_t constant_buffers;
		bound_resources_t shader_resources;
	};







}
