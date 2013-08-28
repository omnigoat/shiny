#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/command.hpp>

#include <atma/lockfree/queue.hpp>

#include <vector>

//======================================================================
// externs
//======================================================================
std::thread shiny::voodoo::detail::prime_thread_;
std::atomic_bool shiny::voodoo::detail::prime_thread_running_;
shiny::voodoo::prime_thread::detail::command_queue_t shiny::voodoo::prime_thread::detail::command_queue;
shiny::voodoo::prime_thread::detail::command_queue_t shiny::voodoo::prime_thread::detail::shutdown_queue;


//======================================================================
// prime_thread
//======================================================================
auto shiny::voodoo::prime_thread::spawn() -> void
{
	using voodoo::detail::prime_thread_;
	using voodoo::detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
	prime_thread_running_ = true;

	prime_thread_ = std::thread([]
	{
		// continue performing all commands always and forever
		while (prime_thread_running_) {
			std::function<void()> x;
			while (detail::command_queue.pop(x)) {
				x();
			}
		}

		// get shutdown commands in LIFO order
		std::vector<std::function<void()>> reversed_commands;
		{
			std::function<void()> x;
			while (detail::shutdown_queue.pop(x))
				reversed_commands.push_back(x);
		}

		// perform all shutdown commands
		for (auto const& x : reversed_commands)
			x();
	});



	// tell the prime-thread to shut down the d3d-device on close
	detail::shutdown_queue.push(&teardown_d3d_device);

	// setup the d3d-device and then block until that's done
	prime_thread::enqueue(&setup_d3d_device);
	prime_thread::enqueue_block();
}

auto shiny::voodoo::prime_thread::detail::reenter(std::atomic_bool const& blocked) -> void
{
	// continue performing all commands always and forever
	std::function<void()> x;
	while (blocked && detail::command_queue.pop(x)) {
		x();
	}
}

auto shiny::voodoo::prime_thread::join() -> void
{
	using voodoo::detail::prime_thread_;
	using voodoo::detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
	prime_thread_running_ = false;
	prime_thread_.join();
}

