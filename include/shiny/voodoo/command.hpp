#ifndef SHINY_VOODOO_COMMAND_HPP
#define SHINY_VOODOO_COMMAND_HPP
//======================================================================
#include <condition_variable>
#include <atma/intrusive_ptr.hpp>
//======================================================================
#include <shiny/voodoo/device.hpp>
//======================================================================
namespace shiny {
namespace voodoo {
//======================================================================
	
	struct command_t : atma::ref_counted
	{
		command_t() {}
		virtual ~command_t() {}
		
		virtual auto operator ()() -> void = 0;

		std::atomic_bool processed;
	};

	typedef atma::intrusive_ptr<command_t> command_ptr;

	template <typename T, typename... Args>
	command_ptr make_command(Args&&... args) {
		return command_ptr(new T(std::forward<Args>(args)...));
	}

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
