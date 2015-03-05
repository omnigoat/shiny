#include <shiny/vertex_declaration.hpp>

#include <shiny/vertex_shader.hpp>


using namespace shiny;
using shiny::vertex_stream_t;
using shiny::vertex_streams_t;
using shiny::vertex_declaration_t;


vertex_declaration_t::vertex_declaration_t(vertex_streams_t const& streams)
: streams_(streams), stride_()
{
	for (auto const& x : streams_)
		stride_ += x.size();
}

auto vertex_declaration_t::streams() const -> vertex_streams_t const&
{
	return streams_;
}

auto vertex_declaration_t::stride() const -> uint
{
	return stride_;
}
