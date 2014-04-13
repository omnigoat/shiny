#include <dust/platform/win32/texture3d.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::texture3d_t;


//======================================================================
// create_texture3d
//======================================================================
auto dust::create_texture3d(context_ptr const& context, surface_format_t format, uint mips, uint width, uint height, uint depth) -> texture3d_ptr
{
	return texture3d_ptr(new texture3d_t(context, {}, format, mips, width, height, depth));
}


//======================================================================
// texture3d
//======================================================================
texture3d_t::texture3d_t(context_ptr const& context, resource_usage_flags_t usage_flags, surface_format_t format, uint mips, uint width, uint height, uint depth)
: resource_t(context, usage_flags), format_(format), mips_(mips), width_(width), height_(height), depth_(depth)
{
	this->context()->create_d3d_texture3d(d3d_texture_, format_, mips_, width_, height_, depth_);
}

auto texture3d_t::format() const -> surface_format_t
{
	return format_;
}

auto texture3d_t::mips() const -> uint
{
	return mips_;
}

auto texture3d_t::width() const -> uint
{
	return width_;
}

auto texture3d_t::height() const -> uint
{
	return height_;
}

auto texture3d_t::depth() const -> uint
{
	return depth_;
}

auto texture3d_t::d3d_texture() const -> platform::d3d_texture3d_ptr const&
{
	return d3d_texture_;
}

auto texture3d_t::d3d_texture() -> platform::d3d_texture3d_ptr&
{
	return d3d_texture_;
}

