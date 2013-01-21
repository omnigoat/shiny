#ifndef SHINY_PLUMBING_COMMAND_HPP
#define SHINY_PLUMBING_COMMAND_HPP
//======================================================================
#include <shiny/plumbing/device.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct command_t
	{
		command_t() {}
		virtual ~command_t() {}
		
		virtual void operator ()() = 0;
	};

	struct map_resource_command_t : command_t
	{
		map_resource_command_t(ID3D11Resource* resource, unsigned int subresource, D3D11_MAP map_type, unsigned int map_flags,
		  D3D11_MAPPED_SUBRESOURCE* mapped_resource)
		: resource(resource), subresource(subresource), map_type(map_type), map_flags(map_flags), mapped_resource(mapped_resource)
		{
		}

		void operator ()() {
			detail::d3d_immediate_context_->Map(resource, subresource, map_type, map_flags, mapped_resource);
		}

		ID3D11Resource* resource;
		unsigned int subresource;
		D3D11_MAP map_type;
		unsigned int map_flags;
		D3D11_MAPPED_SUBRESOURCE* mapped_resource;
	};

	struct copy_data_command_t : command_t
	{
		copy_data_command_t(char* in, unsigned int size, char* out)
		 : in(in), size(size), out(out)
		{
		}

		void operator ()() {
			std::copy_n(in, size, out);
		}

		char* in;
		unsigned int size;
		char* out;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
