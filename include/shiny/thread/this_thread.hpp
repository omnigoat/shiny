#ifndef SHINY_VOODOO_THREAD_THIS_THREAD_HPP
#define SHINY_VOODOO_THREAD_THIS_THREAD_HPP
//======================================================================
#include <thread>
#include <atomic>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <atma/assert.hpp>
#include <atma/lockfree/queue.hpp>
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/command.hpp>
//======================================================================
namespace shiny {
namespace this_thread {
//======================================================================

	auto is_prime() -> bool

//======================================================================
} // namespace this_thread
} // namespace shiny
//======================================================================
#endif
//======================================================================
