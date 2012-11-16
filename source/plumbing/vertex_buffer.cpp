#include <shiny/plumbing/vertex_buffer.hpp>


void shiny::plumbing::vertex_buffer_t::reset_from_shadow_buffer()
{
	
}

shiny::plumbing::vertex_buffer_t::vertex_buffer_t( usage use, void* data, unsigned int data_size, bool shadow )
 : use_(use), data_(data), data_size_(data_size), shadowing_(shadow)
{

}

shiny::plumbing::vertex_buffer_t::lock_t::lock_t(lock_t&& rhs)
 : owner_(rhs.owner_), data_(rhs.data_), data_size_(rhs.data_size_)
{
	rhs.owner_ = nullptr;
	rhs.data_ = nullptr;
	rhs.data_size_ = 0;
}

shiny::plumbing::vertex_buffer_t::lock_t::lock_t( vertex_buffer_t* owner, void* data, unsigned int data_size )
 : owner_(owner), data_(data), data_size_(data_size)
{
}
