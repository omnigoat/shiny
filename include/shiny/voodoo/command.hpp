#ifndef SHINY_VOODOO_COMMAND_HPP
#define SHINY_VOODOO_COMMAND_HPP
//======================================================================
#include <condition_variable>
#include <memory>
#include <atma/intrusive_ptr.hpp>
#include <atma/xtm/tuple.hpp>
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
	//typedef std::shared_ptr<command_t> command_ptr;



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
		{
		}

		auto operator ()() -> void
		{
			(c_.*fn_)();
		}

		R(C::*fn_)(Args...);
		C& c_;
	};






	template <typename R, typename... Params>
	struct bound_fnptr_command_t : command_t
	{
		template <typename... Args>
		bound_fnptr_command_t(R (*fn)(Params...), Args&&... args)
		 : fn_(fn), args_(std::make_tuple(args...))
		{
		}

		auto operator ()() -> void {
			atma::xtm::apply_tuple(fn_, std::forward<std::tuple<Params...>>(args_));
		}

		R (*fn_)(Params...);
		std::tuple<Params...> args_;
	};

	template <typename R>
	struct function_command_t : command_t
	{
		function_command_t( std::function<R()> fn )
		 : fn_(fn)
		{
		}

		auto operator ()() -> void {
			fn_();
		}

		std::function<R()> fn_;
	};

	
	#if 0
	template <typename R, typename... Args>
	inline command_ptr make_command(R(*fn)(Args...))
	{
		return command_ptr(new fnptr_command_t<R, Args...>(fn));
	}
	#endif

	template <typename R, typename... Params, typename... Args>
	inline command_ptr make_command(R(*fn)(Params...), Args&&... args)
	{
		return command_ptr(new bound_fnptr_command_t<R, Params...>(fn, (std::forward<Args>(args))...));
	}

	inline command_ptr make_command(std::function<void()> fn)
	{
		return command_ptr(new function_command_t<void>(fn));
	}

	
	template <typename R, typename C, typename... Args>
	inline command_ptr make_command(R(C::*fn)(Args...), C& c)
	{
		return command_ptr(new memfnptr_command_t<R, C, Args...>(fn, c));
	}
	
//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
