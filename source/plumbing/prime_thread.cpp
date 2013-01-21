#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/device.hpp>
#include <iostream>
auto shiny::plumbing::prime_thread::spawn() -> void
{
	using namespace shiny::plumbing::detail;

	ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
	prime_thread_running_ = true;

	prime_thread_ = std::thread([]{
		std::cout << "weee" << std::endl;

		while (prime_thread_running_.load()) {
			command_t* x = nullptr;
			while (command_queue_.pop(x)) {
				(*x)();
				delete x;
			}
		}
	});
}

auto shiny::plumbing::prime_thread::join() -> void
{
	ATMA_ASSERT(detail::prime_thread_.get_id() != std::thread::id());
	detail::prime_thread_running_.store(false);
	detail::prime_thread_.join();
}

auto shiny::plumbing::prime_thread::submit_command(shiny::plumbing::command_t* c) -> void
{
	detail::command_queue_.push(c);
}

auto shiny::plumbing::prime_thread::submit_command_queue(shiny::plumbing::command_queue_t& q) -> void
{
	command_t* x = nullptr;
	while (q.pop(x))
		detail::command_queue_.push(x);
}
