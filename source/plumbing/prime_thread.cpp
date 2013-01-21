#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/device.hpp>
	
auto shiny::plumbing::detail::spawn_prime_thread() -> void
{
	ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
	prime_thread_running_ = true;

	prime_thread_ = std::thread([]{
		command_t* x = nullptr;
		while (prime_thread_running_.load() && command_queue_.pop(x)) {
			(*x)();
			delete x;
		}
	});
}

auto shiny::plumbing::detail::join_prime_thread() -> void
{
	ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
	prime_thread_running_.store(false);
	prime_thread_.join();
}

auto shiny::plumbing::submit_command(shiny::plumbing::command_t* c) -> void
{
	detail::command_queue_.push(c);
}

auto shiny::plumbing::submit_command_queue(shiny::plumbing::command_queue_t& q) -> void
{
	command_t* x = nullptr;
	while (q.pop(x))
		detail::command_queue_.push(x);
}
