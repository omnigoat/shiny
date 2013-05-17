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
	};

	typedef atma::intrusive_ptr<command_t> command_ptr;



	template <typename R, typename... Args>
	struct fnptr_command_t : command_t
	{
		fnptr_command_t(R(*fn)(Args...))
		 : fn_(fn)
		  {}

		auto operator ()() -> void {
			(*fn_)();
		}

		R (*fn_)(Args...);
	};

	template <typename R, typename C, typename... Args>
	struct memfnptr_command_t : command_t
	{
		memfnptr_command_t(R(C::*fn)(Args...), C& c)
		 : fn_(fn), c_(c)
		  {}

		auto operator ()() -> void {
			(c_.*fn_)();
		}

		R (C::*fn_)(Args...);
		C& c_;
	};

	

	template <typename R, typename... Args>
	command_ptr make_command(R (*fn)(Args...)) {
		return command_ptr(new fnptr_command_t<R, Args...>(fn));
	}

	template <typename R, typename C, typename... Args>
	command_ptr make_command(R (C::*fn)(Args...), C& c) {
		return command_ptr(new memfnptr_command_t<R, C, Args...>(fn, c));
	}

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
