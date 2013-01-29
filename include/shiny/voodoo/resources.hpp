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
	
//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
