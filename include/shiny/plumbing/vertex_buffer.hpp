#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct vertex_buffer_t
	{
		vertex_buffer_t(void* data, unsigned int data_size, bool shadow)
			: hardware_buffer_t(data, data_size, shadow)
		{
		}
	
	private:
		
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
