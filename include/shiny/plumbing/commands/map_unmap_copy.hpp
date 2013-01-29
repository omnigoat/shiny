#ifndef SHINY_PLUMBING_COMMANDS_MAP_UNMAP_COPY_HPP
#define SHINY_PLUMBING_COMMANDS_MAP_UNMAP_COPY_HPP
//======================================================================
#include <shiny/voodoo/command.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
namespace commands {
//======================================================================
	
	struct map : voodoo::command_t
	{
		map(ID3D11Resource* resource, unsigned int subresource, D3D11_MAP map_type, unsigned int map_flags,
		  D3D11_MAPPED_SUBRESOURCE* mapped_resource)
		: resource(resource), subresource(subresource), map_type(map_type), map_flags(map_flags), mapped_resource(mapped_resource)
		{
		}

		void operator ()() {
			//detail::d3d_immediate_context_->Map(resource, subresource, map_type, map_flags, mapped_resource);
		}

		ID3D11Resource* resource;
		unsigned int subresource;
		D3D11_MAP map_type;
		unsigned int map_flags;
		D3D11_MAPPED_SUBRESOURCE* mapped_resource;
	};

	struct unmap : voodoo::command_t
	{
		unmap(ID3D11Resource* resource, unsigned int subresource)
		: resource(resource), subresource(subresource)
		{
		}

		void operator ()() {
			//detail::d3d_immediate_context_->Unmap(resource, subresource);
		}

		ID3D11Resource* resource;
		unsigned int subresource;
	};

	struct copy : voodoo::command_t
	{
		copy(char* in, unsigned int size, char* out)
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
} // namespace commands
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
