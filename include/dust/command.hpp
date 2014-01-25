#ifndef DUSK_VOODOO_COMMAND_HPP
#define DUSK_VOODOO_COMMAND_HPP
//======================================================================
#include <condition_variable>
#include <memory>
#include <atma/intrusive_ptr.hpp>
#include <atma/xtm/tuple.hpp>
//======================================================================
#include <dusk/device.hpp>
//======================================================================
namespace dusk {
namespace voodoo {
//======================================================================
	

	//======================================================================
	// command_t
	// -----------
	//   something sent to the prime_thread
	//======================================================================
	struct command_t : atma::ref_counted<command_t>
	{
		command_t() {}
		virtual ~command_t() {}
		virtual auto operator ()() -> void = 0;
	};

	typedef atma::intrusive_ptr<command_t> command_ptr;
	

	//======================================================================
	// ...
	//======================================================================
	namespace detail
	{
		template <typename R, typename... Params>
		struct bound_fnptr_command_t : command_t
		{
			template <typename... Args>
			bound_fnptr_command_t(R(*fn)(Params...), Args&&... args)
			 : fn_(fn), args_(std::make_tuple(args...))
			{
			}

			auto operator ()() -> void
			{
				atma::xtm::apply_tuple(fn_, std::forward<std::tuple<Params...>>(args_));
			}

			R(*fn_)(Params...);
			std::tuple<Params...> args_;
		};

		template <typename R, typename C, typename... Params>
		struct bound_memfnptr_command_t : command_t
		{
			template <typename... Args>
			bound_memfnptr_command_t(R(C::*fn)(Params...), C* c, Args&&... args)
			 : fn_(fn), args_(std::make_tuple(c, args...))
			{
			}

			auto operator ()() -> void
			{
				atma::xtm::apply_tuple(fn_, std::forward<std::tuple<C*, Params...>>(args_));
			}

			R(C::*fn_)(Params...);
			std::tuple<C*, Params...> args_;
		};

		template <typename F>
		struct bound_callable_command_t : command_t
		{
			bound_callable_command_t(F&& fn)
			 : fn_(fn)
			{
			}

			auto operator ()() -> void
			{
				fn_();
			}

			F fn_;
		};
	}




	//======================================================================
	// make_command
	//======================================================================
	// function-pointer
	template <typename R, typename... Params, typename... Args>
	inline auto make_command(R(*fn)(Params...), Args&&... args) -> command_ptr {
		return command_ptr(new detail::bound_fnptr_command_t<R, Params...>(fn, (std::forward<Args>(args))...));
	}

	template <typename R, typename... Params>
	inline auto make_command(R(*fn)(Params...)) -> command_ptr {
		return command_ptr(new detail::bound_fnptr_command_t<R, Params...>(fn));
	}

	// member-function-pointer
	template <typename R, typename C, typename... Params, typename... Args>
	inline auto make_command(R(C::*fn)(Params...), C* c, Args&&... args) -> command_ptr {
		return command_ptr(new detail::bound_memfnptr_command_t<R, C, Params...>
			(fn, c, (std::forward<Args>(args))...));
	}

	template <typename R, typename C, typename... Params>
	inline auto make_command(R(C::*fn)(Params...), C* c) -> command_ptr {
		return command_ptr(new detail::bound_memfnptr_command_t<R, C, Params...>(fn, c));
	}

	// whatevs.
	template <typename F>
	inline auto make_command(F&& fn) -> command_ptr {
		return command_ptr(new detail::bound_callable_command_t<F>(std::forward<F>(fn)));
	}

	
	
	
//======================================================================
} // namespace voodoo
} // namespace dusk
//======================================================================
#endif
//======================================================================
