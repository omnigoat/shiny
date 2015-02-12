#include <dust/vertex_declaration.hpp>

#include <dust/vertex_shader.hpp>


using namespace dust;
using dust::vertex_stream_t;
using dust::vertex_streams_t;
using dust::vertex_declaration_t;


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
