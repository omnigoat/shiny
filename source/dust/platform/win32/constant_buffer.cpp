#include <dust/platform/win32/constant_buffer.hpp>

#include <dust/context.hpp>

#include <atma/math/matrix4f.hpp>

#include <DirectXMath.h>

using namespace dust;
using dust::constant_buffer_t;


constant_buffer_t::constant_buffer_t(context_ptr const& context)
: context_(context)
{
	struct B
	{
		atma::math::matrix4f world;
		atma::math::matrix4f view;
		atma::math::matrix4f proj;
		float time;
	};

	B b;
	static float t = 1.f;
	
	b.time = t;
	t += 0.1f;

	namespace math = atma::math;
	static float x = 0.f;
	static float y = 0.f;
	if (GetAsyncKeyState(VK_LEFT))
		x -= 0.001f;
	else if (GetAsyncKeyState(VK_RIGHT))
		x += 0.001f;
	else if (GetAsyncKeyState(VK_UP))
		y += 0.001f;
	else if (GetAsyncKeyState(VK_DOWN))
		y -= 0.001f;
		
	using namespace DirectX;
	auto M = XMMatrixLookAtLH(XMVectorSet(sin(x) * cos(y) * 2.f, sin(y) * 2.f, cos(x) * cos(y) * 2.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 0.f), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	auto P = XMMatrixPerspectiveFovLH(XM_PIDIV2, 480.f / 360.f, 0.01f, 100.f);

	//M = XMMatrixTranspose(M);
	P = XMMatrixTranspose(P);

	//b.view = math::look_at(math::vector4f(x, 1.f, -1.f, 0.f), math::vector4f(0.f, 0.f, 0.f, 0.f), math::vector4f(0.f, 1.f, 0.f, 0.f));
	b.view = math::matrix4f(M.r[0], M.r[1], M.r[2], M.r[3]);

	b.proj = math::perspective_fov(XM_PIDIV2, 480.f / 360.f, 0.01f, 100.f);

	auto R = XMMatrixTranspose(XMMatrixRotationY(t * 0.002f));
	b.world = math::matrix4f(R.r[0], R.r[1], R.r[2], R.r[3]);
	
	
	//b.view = b.view;
	//b.proj = b.proj; // math::matrix4f(P.r[0], P.r[1], P.r[2], P.r[3]);
	//b.proj[0][0] = 0.7f;
	//b.proj[1][1] = 1.f;

	context_->create_d3d_buffer(d3d_buffer_,
		buffer_type_t::constant_buffer, gpu_access_t::read, cpu_access_t::write,
		sizeof(B), &b);
}

auto dust::create_constant_buffer(context_ptr const& context) -> constant_buffer_ptr
{
	return constant_buffer_ptr(new constant_buffer_t(context));
}