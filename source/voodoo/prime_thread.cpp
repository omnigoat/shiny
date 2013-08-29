#include <shiny/voodoo/prime_thread.hpp>

#include <vector>
#include <functional>

using namespace shiny;

//======================================================================
// externs
//======================================================================
std::thread shiny::voodoo::prime_thread::detail::handle_;
std::atomic_bool shiny::voodoo::prime_thread::detail::is_running_;
shiny::voodoo::prime_thread::detail::command_queue_t shiny::voodoo::prime_thread::detail::command_queue_;
shiny::voodoo::prime_thread::detail::command_queue_t shiny::voodoo::prime_thread::detail::shutdown_queue_;




//======================================================================
// prime_thread
//======================================================================
auto shiny::voodoo::prime_thread::spawn() -> void
{
	ATMA_ASSERT(detail::handle_.get_id() == std::thread::id());
	detail::is_running_ = true;

	detail::handle_ = std::thread([]
	{
		// continue performing all commands always and forever
		while (detail::is_running_) {
			std::function<void()> x;
			while (detail::command_queue_.pop(x)) {
				x();
			}
		}

		// get shutdown commands in LIFO order
		std::vector<std::function<void()>> reversed_commands;
		{
			std::function<void()> x;
			while (detail::shutdown_queue_.pop(x))
				reversed_commands.push_back(x);
		}

		// perform all shutdown commands
		for (auto const& x : reversed_commands)
			x();
	});


	// tell the prime-thread to shut down the d3d-device on close
	detail::shutdown_queue_.push(&teardown_d3d_device);

	// setup the d3d-device and then block until that's done
	prime_thread::enqueue(&setup_d3d_device);
	prime_thread::enqueue_block();
}

auto shiny::voodoo::prime_thread::join() -> void
{
	ATMA_ASSERT(detail::handle_.get_id() != std::thread::id());
	detail::is_running_ = false;
	detail::handle_.join();
}

auto shiny::voodoo::prime_thread::detail::reenter(std::atomic_bool const& blocked) -> void
{
	// continue performing all commands always and forever
	std::function<void()> x;
	while (blocked && detail::command_queue_.pop(x)) {
		x();
	}
}

auto shiny::voodoo::prime_thread::enqueue(std::function<void()> const& fn) -> void
{
	if (!detail::is_running_)
		return;

	detail::command_queue_.push(fn);
}

auto shiny::voodoo::prime_thread::enqueue_block() -> void
{
	if (!detail::is_running_)
		return;

	// push blocking fn!
	std::atomic_bool blocked{ true };
	detail::command_queue_.push([&blocked]{ blocked = false; });

	// don't block if we're the prime thread blocking ourselves.
	if (std::this_thread::get_id() == detail::handle_.get_id())
	{
		detail::reenter(blocked);
		return;
	}
	else
	{
		while (blocked)
			;
	}
}


