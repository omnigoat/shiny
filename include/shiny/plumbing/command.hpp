#ifndef SHINY_PLUMBING_COMMAND_HPP
#define SHINY_PLUMBING_COMMAND_HPP
//======================================================================
#include <condition_variable>
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

	struct wakeup_command_t : command_t
	{
		wakeup_command_t(bool& woken, std::condition_variable& cv)
		: woken(woken), cv(cv)
		{
		}

		void operator ()() {
			woken = true;
			cv.notify_all();
		}

		bool& woken;
		std::condition_variable& cv;
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

	struct unmap_resource_command_t : command_t
	{
		unmap_resource_command_t(ID3D11Resource* resource, unsigned int subresource)
		: resource(resource), subresource(subresource)
		{
		}

		void operator ()() {
			detail::d3d_immediate_context_->Unmap(resource, subresource);
		}

		ID3D11Resource* resource;
		unsigned int subresource;
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

	struct callback_command_t : command_t
	{
		callback_command_t(std::function<void()> const& fn) : fn(fn) {}

		void operator ()() {
			fn();
		}

		std::function<void()> fn;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
