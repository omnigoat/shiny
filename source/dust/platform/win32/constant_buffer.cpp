#include <dust/platform/win32/constant_buffer.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::constant_buffer_t;


constant_buffer_t::constant_buffer_t(context_ptr const& context, uint data_size, void* data)
: context_(context), data_size_(data_size)
{
	context_->create_d3d_buffer(d3d_buffer_,
		buffer_type_t::constant_buffer, gpu_access_t::read, cpu_access_t::write,
		data_size_, data);
}

auto constant_buffer_t::data_size() const -> uint
{
	return data_size_;
}

auto constant_buffer_t::signal_upload_new_data(void* data) -> void
{
	D3D11_MAPPED_SUBRESOURCE dmap{data, data_size_, 1};

	std::shared_ptr<char> data_copy(new char[data_size_], std::default_delete<char[]>());
	memcpy(data_copy.get(), data, data_size_);

	auto data_size = this->data_size_;
	context_->signal_d3d_map(d3d_buffer_, &dmap, D3D11_MAP_WRITE_DISCARD, 0, [data_copy, data_size](D3D11_MAPPED_SUBRESOURCE* dmap) {
		memcpy(dmap->pData, data_copy.get(), data_size);
	});
}

auto dust::create_constant_buffer(context_ptr const& context, uint data_size, void* data) -> constant_buffer_ptr
{
	return constant_buffer_ptr(new constant_buffer_t(context, data_size, data));
}