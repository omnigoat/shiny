#include <dust/platform/win32/texture2d.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::texture2d_t;


//======================================================================
// create_texture2d
//======================================================================
auto dust::create_texture2d(context_ptr const& context, resource_usage_flags_t flags, surface_format_t format, uint width, uint height) -> texture2d_ptr
{
	return texture2d_ptr(new texture2d_t(context, flags, format, 0, width, height));
}

auto dust::create_texture2d(context_ptr const& context, surface_format_t format, uint width, uint height) -> texture2d_ptr
{
	return create_texture2d(context, {}, format, width, height);
}


//======================================================================
// texture2d
//======================================================================
texture2d_t::texture2d_t(context_ptr const& context, resource_usage_flags_t usage_flags, surface_format_t format, uint width, uint height, uint mips)
: resource_t(context, usage_flags), format_(format), width_(width), height_(height), mips_(mips)
{
	this->context()->create_d3d_texture2d(d3d_texture_, usage_flags, format_, width_, height_, mips_);
}

auto texture2d_t::format() const -> surface_format_t
{
	return format_;
}

auto texture2d_t::width() const -> uint
{
	return width_;
}

auto texture2d_t::height() const -> uint
{
	return height_;
}

auto texture2d_t::mips() const -> uint
{
	return mips_;
}

auto texture2d_t::d3d_texture() const -> platform::d3d_texture2d_ptr const&
{
	return d3d_texture_;
}

auto texture2d_t::d3d_texture() -> platform::d3d_texture2d_ptr&
{
	return d3d_texture_;
}

