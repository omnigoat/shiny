#include <dust/vertex_declaration.hpp>
#include <d3dcompiler.h>

using namespace dust;
using dust::vertex_stream_t;
using dust::vertex_declaration_t;

//======================================================================
// vertex_stream_t
//======================================================================
vertex_stream_t::vertex_stream_t(usage_t usage, uint32_t index, element_type_t element_type, uint32_t element_count)
	: usage_(usage), index_(index), element_type_(element_type), element_count_(element_count)
{
}

auto vertex_stream_t::usage() const -> usage_t {
	return usage_;
}

auto vertex_stream_t::index() const -> uint32_t {
	return index_;
}

auto vertex_stream_t::element_type() const -> element_type_t {
	return element_type_;
}

auto vertex_stream_t::element_count() const -> uint32_t {
	return element_count_;
}

auto vertex_stream_t::size() const -> uint32_t
{
	uint32_t element_size = 0;
	switch (element_type_)
	{
		case element_type_t::float32:
		case element_type_t::int32:
		case element_type_t::uint32:
			element_size = 4;
			break;

		case element_type_t::int16:
		case element_type_t::uint16:
			element_size = 2;
			break;

		case element_type_t::int8:
		case element_type_t::uint8:
			element_size = 1;
			break;
	}

	ATMA_ASSERT(element_size > 0);
	
	return element_size * element_count_;
}




//======================================================================
// vertex_declaration_t
//======================================================================
vertex_declaration_t::vertex_declaration_t( context_ptr const& context, std::initializer_list<vertex_stream_t> streams )
: context_(context), streams_(streams.begin(), streams.end()), stride_(), d3d_input_layout_(), built_()
{
	// calculate stride
	for (auto const& x : streams_)
		stride_ += x.size();

	build();
}

auto vertex_declaration_t::streams() const -> streams_t const&
{
	return streams_;
}

auto vertex_declaration_t::stride() const -> uint32_t
{
	return stride_;
}

auto vertex_declaration_t::d3d_input_layout() const -> platform::d3d_input_layout_ptr const&
{
	return d3d_input_layout_;
}

auto vertex_declaration_t::build() -> void
{
	if (built_)
		return;

	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	//for (auto const& x : streams_)
		d3d_elements.push_back({
			"position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
		});

	
	ID3DBlob* blob;
	ATMA_ENSURE_IS(S_OK, D3DCompileFromFile(L"../shaders/vs_basic.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &blob, nullptr));

	ID3D11VertexShader* vs = nullptr;
	
	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vs));
	
	ATMA_ENSURE_IS(S_OK, device->CreateInputLayout(&d3d_elements[0], (uint32_t)d3d_elements.size(),
		blob->GetBufferPointer(), blob->GetBufferSize(), d3d_input_layout_.assign()));

	built_ = true;
}
