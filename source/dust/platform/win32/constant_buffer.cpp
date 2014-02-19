#include <dust/platform/win32/constant_buffer.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::constant_buffer_t;


constant_buffer_t::constant_buffer_t(context_ptr const& context)
: context_(context)
{
	struct B
	{
		atma::math::matrix4f view;
		atma::math::matrix4f proj;
		float time;
	};

	B b;
	static float t = 1.f;
	b.time = t;
	t += 0.1f;

	//b.time = (float)std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

	context_->create_d3d_buffer(d3d_buffer_,
		buffer_type_t::constant_buffer, gpu_access_t::read, cpu_access_t::write,
		sizeof(B), &b);
}

auto dust::create_constant_buffer(context_ptr const& context) -> constant_buffer_ptr
{
	return constant_buffer_ptr(new constant_buffer_t(context));
}