#ifndef DUSK_VOODOO_RESOURCES_HPP
#define DUSK_VOODOO_RESOURCES_HPP
//======================================================================
#include <thread>
#include <atomic>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace dusk {
namespace voodoo {
//======================================================================

	enum class gpu_access_t
	{
		read,
		write,
		read_write
	};

	enum class cpu_access_t
	{
		none,
		read,
		write,
		read_write,
	};

	auto create_buffer(ID3D11Buffer**, gpu_access_t, cpu_access_t, uint32_t data_size, void* data = nullptr) -> void;
	auto create_index_buffer(ID3D11Buffer**, gpu_access_t, cpu_access_t, uint32_t data_size, void* data = nullptr) -> void;
	auto map(ID3D11Resource*, D3D11_MAPPED_SUBRESOURCE*, D3D11_MAP, uint32_t subresource) -> void;
	auto map_vb(ID3D11Buffer*, D3D11_MAPPED_SUBRESOURCE*, D3D11_MAP, uint32_t subresource) -> void;
	auto unmap(ID3D11Resource*, uint32_t subresource) -> void;

//======================================================================
} // namespace voodoo
//======================================================================

	typedef voodoo::gpu_access_t gpu_access_t;
	typedef voodoo::cpu_access_t cpu_access_t;

} // namespace dusk
//======================================================================
#endif
//======================================================================
