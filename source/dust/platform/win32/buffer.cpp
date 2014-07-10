#include <dust/platform/win32/buffer.hpp>

#include <dust/platform/win32/dxgid3d_convert.hpp>
#include <dust/context.hpp>
#include <dust/vertex_declaration.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::buffer_t;

buffer_t::buffer_t(context_ptr const& ctx, buffer_type_t type, buffer_usage_t usage, uint element_size, uint element_count, void const* data, uint data_element_count)
: resource_t(ctx, {}), type_(type), usage_(usage), size_(element_size * element_count)
{
	ATMA_ASSERT(size_);


	// fixup default element-count, figure out size of data
	if (data_element_count == 0)
		data_element_count = element_count;
	auto data_size = element_size * data_element_count;


	// determine buffer-usage and cpu-access
	auto d3d_bu = D3D11_USAGE();
	auto d3d_ca = D3D11_CPU_ACCESS_FLAG();
	switch (usage_)
	{
		case buffer_usage_t::immutable:
			d3d_bu = D3D11_USAGE_IMMUTABLE;
			break;

		case buffer_usage_t::long_lived:
		case buffer_usage_t::long_lived_shadowed:
			d3d_bu = D3D11_USAGE_DEFAULT;
			d3d_ca = D3D11_CPU_ACCESS_WRITE;
			break;

		case buffer_usage_t::dynamic:
		case buffer_usage_t::dynamic_shadowed:
			d3d_bu = D3D11_USAGE_DYNAMIC;
			d3d_ca = D3D11_CPU_ACCESS_WRITE;
			break;

		default:
			ATMA_HALT("buffer usage is ill-formed");
			break;
	}


	// determine special flags based off buffer-type
	auto misc_flags = D3D11_RESOURCE_MISC_FLAG();
	switch (type_)
	{
		case buffer_type_t::generic_buffer:
			(uint&)misc_flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;
	}


	// allocate shadow-buffer if need be
	switch (usage_)
	{
		case buffer_usage_t::long_lived_shadowed:
		case buffer_usage_t::dynamic_shadowed:
		{
			shadow_buffer_.resize((uint)size_);
			if (data)
				memcpy(&shadow_buffer_[0], data, data_size);
			break;
		}

		default:
			break;
	}


	// create buffer
	auto buffer_desc = D3D11_BUFFER_DESC{(UINT)size_, d3d_bu, platform::d3dbind_of(type_), d3d_ca, misc_flags, element_size};
	switch (usage_)
	{
		case buffer_usage_t::immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");
			ATMA_ASSERT_MSG(size_ == data_size, "immutable buffer: you are allocating more than you're filling");
			ATMA_ASSERT_MSG(d3d_ca == 0, "immutable buffer with cpu access? silly.");

			auto d3d_data = D3D11_SUBRESOURCE_DATA{data, (UINT)data_size, 1};
			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			break;
		}

		case buffer_usage_t::long_lived:
		case buffer_usage_t::dynamic:
		{
			if (data) {
				auto d3d_data = D3D11_SUBRESOURCE_DATA{data, (UINT)data_size, 1};
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			}
			else {
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, nullptr, d3d_buffer_.assign()));
			}

			break;
		}

		case buffer_usage_t::long_lived_shadowed:
		case buffer_usage_t::dynamic_shadowed:
		{
			if (data) {
				auto d3d_data = D3D11_SUBRESOURCE_DATA{&shadow_buffer_[0], (UINT)data_size, 1};
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			}
			else {
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, nullptr, d3d_buffer_.assign()));
			}

			break;
		}
	}
}


buffer_t::~buffer_t()
{
}

auto buffer_t::upload_shadow_buffer() -> void
{
	ATMA_ASSERT(is_shadowing());

	context()->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	context()->signal_block();
}

auto buffer_t::d3d_resource() const -> platform::d3d_resource_ptr
{
	return d3d_buffer_;
}

auto buffer_t::d3d_srv() const -> platform::d3d_shader_resource_view_ptr const&
{
	return d3d_srv_;
}