#include <dust/resources.hpp>
#include <dust/device.hpp>
#include <atma/assert.hpp>

using dust::voodoo::gpu_access_t;
using dust::voodoo::cpu_access_t;

namespace {
	uint32_t const ptr_size = sizeof(ID3D11Buffer*);
	GUID const mapped_context_guid = { 0x01, 0x02, 0x03, "ctx_map" };
}

auto dust::voodoo::create_buffer(ID3D11Buffer** buffer, gpu_access_t gpu_access, cpu_access_t cpu_access, uint32_t data_size, void* data) -> void
{
	// calcualte the buffer usage based off our gpu-access/cpu-access flags
	D3D11_USAGE buffer_usage = D3D11_USAGE_DEFAULT;
	if (cpu_access == cpu_access_t::write) {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_DYNAMIC;
		}
		else if (gpu_access == gpu_access_t::write || gpu_access == gpu_access_t::read_write) {
			buffer_usage = D3D11_USAGE_STAGING;
		}
	}
	else if (cpu_access == cpu_access_t::read || cpu_access == cpu_access_t::read_write) {
		buffer_usage = D3D11_USAGE_STAGING;
	}
	else {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_IMMUTABLE;
		}
	}

	// cpu-usage still needs to be calculated
	D3D11_CPU_ACCESS_FLAG cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(0L);
	switch (cpu_access) {
		case cpu_access_t::read: cpua = D3D11_CPU_ACCESS_READ; break;
		case cpu_access_t::write: cpua = D3D11_CPU_ACCESS_WRITE; break;
		case cpu_access_t::read_write: cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE); break;
	}
	
	D3D11_BUFFER_DESC buffer_desc { data_size, buffer_usage, D3D11_BIND_VERTEX_BUFFER, cpua, 0, 0 };

#if 0
	if (data) {
		// for vertex buffers, the pitch and slice-pitch mean nothing, so we'll just
		// pass along stats to help with debugging (1xdata_size buffer created)
		D3D11_SUBRESOURCE_DATA d3d_data { data, 1, data_size };
		ATMA_ENSURE_IS(S_OK, detail::d3d_device_->CreateBuffer(&buffer_desc, &d3d_data, buffer));
	}
	else {
		detail::d3d_device_->CreateBuffer(&buffer_desc, NULL, buffer);
	}
#endif
}



auto dust::voodoo::create_index_buffer(ID3D11Buffer** buffer, gpu_access_t gpu_access, cpu_access_t cpu_access, uint32_t data_size, void* data) -> void
{
	// calcualte the buffer usage based off our gpu-access/cpu-access flags
	D3D11_USAGE buffer_usage = D3D11_USAGE_DEFAULT;
	if (cpu_access == cpu_access_t::write) {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_DYNAMIC;
		}
		else if (gpu_access == gpu_access_t::write || gpu_access == gpu_access_t::read_write) {
			buffer_usage = D3D11_USAGE_STAGING;
		}
	}
	else if (cpu_access == cpu_access_t::read || cpu_access == cpu_access_t::read_write) {
		buffer_usage = D3D11_USAGE_STAGING;
	}
	else {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_IMMUTABLE;
		}
	}

	// cpu-usage still needs to be calculated
	D3D11_CPU_ACCESS_FLAG cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(0L);
	switch (cpu_access) {
		case cpu_access_t::read: cpua = D3D11_CPU_ACCESS_READ; break;
		case cpu_access_t::write: cpua = D3D11_CPU_ACCESS_WRITE; break;
		case cpu_access_t::read_write: cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE); break;
	}

	D3D11_BUFFER_DESC buffer_desc { data_size, buffer_usage, D3D11_BIND_INDEX_BUFFER, cpua, 0, 0 };

#if 0
	if (data) {
		// for vertex buffers, the pitch and slice-pitch mean nothing, so we'll just
		// pass along stats to help with debugging (1xdata_size buffer created)
		D3D11_SUBRESOURCE_DATA d3d_data { data, 1, data_size };
		detail::d3d_device_->CreateBuffer(&buffer_desc, &d3d_data, buffer);
	}
	else {
		detail::d3d_device_->CreateBuffer(&buffer_desc, NULL, buffer);
	}
#endif
}

auto dust::voodoo::map(ID3D11Resource* d3d_resource, D3D11_MAPPED_SUBRESOURCE* d3d_mapped_resource, D3D11_MAP map_type, uint32_t subresource) -> void
{
#if 0
	detail::d3d_local_context_->Map(d3d_resource, subresource, map_type, 0, d3d_mapped_resource);
	ID3D11DeviceContext* mapping_context = detail::d3d_local_context_;

	// store which context mapped the resource
	d3d_resource->SetPrivateData(mapped_context_guid, ptr_size, &detail::d3d_local_context_);
#endif
}

auto dust::voodoo::map_vb(ID3D11Buffer* d3d_resource, D3D11_MAPPED_SUBRESOURCE* d3d_mapped_resource, D3D11_MAP map_type, uint32_t subresource) -> void
{
	map((ID3D11Resource*)d3d_resource, d3d_mapped_resource, map_type, subresource);
}

auto dust::voodoo::unmap(ID3D11Resource* d3d_resource, uint32_t subresource) -> void
{
#if 0
	// use the device stored in private data to assert it's the same thread
	auto size = ptr_size;
	ID3D11DeviceContext* d3d_context = nullptr;
	ATMA_ENSURE_IS(S_OK, d3d_resource->GetPrivateData(mapped_context_guid, &size, &d3d_context));
	ATMA_ASSERT(size == ptr_size);
	ATMA_ASSERT(detail::d3d_local_context_ == d3d_context);

	detail::d3d_local_context_->Unmap(d3d_resource, subresource);
#endif
}





