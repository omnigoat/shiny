#include <dust/platform/win32/vertex_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::vertex_shader_t;


vertex_shader_t::vertex_shader_t(context_ptr const& context)
: context_(context)
{
	ATMA_ENSURE_IS(S_OK, D3DCompileFromFile(L"../shaders/vs_basic.hlsl", nullptr, nullptr, "main", "vs_4_0", 0, 0, d3d_blob_.assign(), nullptr));

	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_vs_.assign()));
}


