#include <dust/vertex_declaration.hpp>

#include <dust/vertex_shader.hpp>


using namespace dust;
using dust::vertex_stream_t;
using dust::vertex_declaration_t;


std::map<vertex_declaration_t::streams_t, std::unique_ptr<vertex_declaration_t>> cache_;

auto dust::get_vertex_declaration(vertex_declaration_t::streams_t const& streams) -> vertex_declaration_t const*
{
	auto i = cache_.find(streams);
	if (i == cache_.end()) {
		auto p = std::unique_ptr<vertex_declaration_t>(new vertex_declaration_t(streams));
		i = cache_.insert(std::make_pair(streams, std::move(p))).first;
	}
	
	return i->second.get();
}

vertex_declaration_t::vertex_declaration_t(streams_t const& streams)
: streams_(streams), stride_()
{
	for (auto const& x : streams_)
		stride_ += x.size();
}

auto vertex_declaration_t::streams() const -> streams_t const&
{
	return streams_;
}

auto vertex_declaration_t::stride() const -> uint
{
	return stride_;
}

#if 0
auto vertex_declaration_t::build(vertex_shader_ptr const& vs) -> void
{
	if (built_)
		return;

	uint offset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	for (auto const& x : streams_)
	{
		d3d_elements.push_back({
			x.usage() == vertex_stream_t::usage_t::position ? "Position" : "Color",
			0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0
		});

		offset += x.size();
	}

	ATMA_ENSURE_IS(S_OK, context_->d3d_device()->CreateInputLayout(&d3d_elements[0], (uint32)d3d_elements.size(),
		vs->d3d_blob()->GetBufferPointer(), vs->d3d_blob()->GetBufferSize(), d3d_input_layout_.assign()));

	built_ = true;
}
#endif
