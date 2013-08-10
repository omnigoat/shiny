#ifndef SHINY_SCOPED_RUNTIME_HPP
#define SHINY_SCOPED_RUNTIME_HPP
//======================================================================
#include <functional>
#include <vector>
#include <thread>
//======================================================================
namespace shiny {
//======================================================================
	
	struct scoped_runtime_t
	{
		scoped_runtime_t();
		~scoped_runtime_t();

		auto add_thread(std::function<void()> fn) -> void;
		auto add_context_thread(std::function<void()> fn) -> void;

	private:
		typedef std::vector<std::thread> threads_t;
		threads_t threads_;
	};

//======================================================================
} // namespace shiny
//======================================================================
#endif
//======================================================================
