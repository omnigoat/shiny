#ifndef SHINY_PLUMBING_COMMAND_HPP
#define SHINY_PLUMBING_COMMAND_HPP
//======================================================================
#include <condition_variable>
#include <atma/intrusive_ptr.hpp>
//======================================================================
#include <shiny/plumbing/device.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct command_t : atma::ref_counted
	{
		command_t() {}
		virtual ~command_t() {}
		
		virtual void operator ()() = 0;

		std::atomic_bool processed;
	};

	typedef atma::intrusive_ptr<command_t> command_ptr;

	template <typename T, typename... Args>
	command_ptr make_command(Args&&... args) {
		return command_ptr(new T(std::forward<Args>(args)...));
	}

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
