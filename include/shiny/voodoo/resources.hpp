#ifndef SHINY_VOODOO_RESOURCES_HPP
#define SHINY_VOODOO_RESOURCES_HPP
//======================================================================
#include <thread>
#include <atomic>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
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

	auto create_buffer(ID3D11Buffer**, gpu_access_t, cpu_access_t, unsigned long data_size, void* data = nullptr) -> void;
	auto map(ID3D11Resource*, D3D11_MAPPED_SUBRESOURCE*, D3D11_MAP, unsigned int subresource) -> void;
	auto unmap(ID3D11Resource*, uint32_t subresource) -> void;

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
