#include <shiny/draw.hpp>

#include <shiny/renderer.hpp>


using namespace shiny;

#if 0
#if 0
auto shiny::detail::generate_draw_prelude(draw_command_t& dc, renderer_ptr const& rndr) -> void
{
	dc.push([rndr]{
		rndr->immediate_draw_pipeline_reset();
	});
}
#endif

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, input_assembly_stage_t const& ias) -> void
{
	dc.ia = ias;
#if 0
	dc.push([rndr, ias]
	{
		rndr->immediate_ia_set_topology(ias.tp);
		rndr->immediate_ia_set_data_declaration(ias.dd);
		rndr->immediate_ia_set_vertex_buffer(ias.vb);

		if (ias.ib)
		{
			rndr->immediate_ia_set_index_buffer(ias.ib);
		}
	});
#endif

}

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, vertex_stage_t const& vs) -> void
{
	dc.push([rndr, vs]{
		rndr->immediate_vs_set_vertex_shader(vs.vs);
		rndr->immediate_vs_set_constant_buffers(vs.cbs);
		rndr->immediate_vs_set_input_views(vs.ivs);
	});
}

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, geometry_stage_t const& gs) -> void
{
	dc.push([rndr, gs] {
		rndr->immediate_gs_set_geometry_shader(gs.gs);
		rndr->immediate_gs_set_constant_buffers(gs.cbs);
	});
}

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, fragment_stage_t const& fs) -> void
{
	dc.push([rndr, fs]{
		rndr->immediate_fs_set_fragment_shader(fs.fs);
		rndr->immediate_fs_set_constant_buffers(fs.cbs);
		rndr->immediate_fs_set_input_views(fs.ivs);
		rndr->immediate_fs_set_compute_views(fs.cvs);
	});
}

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, output_merger_stage_t const& om) -> void
{
	dc.push([rndr, om]{
		rndr->immediate_om_set_blending(om.b);
	});
}

auto shiny::detail::generate_command(draw_command_t& dc, renderer_ptr const& rndr, draw_range_t const& dr) -> void
{
	dc.push([rndr, dr]{
		rndr->immediate_draw_set_range(dr);
	});
}

#if 0
auto shiny::detail::dispatch_signal_draw(renderer_ptr const& rndr, draw_command_t& dc) -> void
{
	dc.push([rndr]{
		rndr->immediate_draw();
	});

	//rndr->signal(dc);
}
#endif

#endif
