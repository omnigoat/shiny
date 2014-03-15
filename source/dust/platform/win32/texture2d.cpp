#include <dust/platform/win32/texture2d.hpp>

#include <dust/context.hpp>

using namespace dust;
using dust::texture2d_t;

auto texture2d_t::create(context_ptr const& context, surface_format_t format, uint width, uint height) -> texture2d_ptr
{
	return texture2d_ptr(new texture2d_t(context, texture_usage_t::normal, format, width, height, 0));
}

texture2d_t::texture2d_t(context_ptr const& context, texture_usage_t usage, surface_format_t format, uint width, uint height, uint mips)
	: context_(context), usage_(usage), format_(format), width_(width), height_(height), mips_(mips)
{
	//context_->create_d3d_texture2d(d3d_texture_, usage_, format_, width_, height_, mips_);
}

auto texture2d_t::usage() const -> texture_usage_t
{
	return usage_;
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

