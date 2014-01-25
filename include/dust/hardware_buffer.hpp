#ifndef DUSK_PLUMBING_HARDWARE_BUFFER_HPP
#define DUSK_PLUMBING_HARDWARE_BUFFER_HPP
//======================================================================
namespace dusk {
namespace plumbing {
//======================================================================
	
	struct hardware_buffer_t
	{
		void reset_from_shadow_buffer() {
			this->reset_from_shadow_buffer_impl();
		}

	protected:
		hardware_buffer_t(buffer_type type, void * data, unsigned int data_size, bool shadow)
			: type_(type), data_(data), data_size_(data_size), shadowing_(shadow)
		{
		}

		virtual ~hardware_buffer_t() {}
		

		virtual void reset_from_shadow_buffer() = 0;

	protected:
		buffer_type type_;
		void* data_;
		unsigned int data_size_;
		bool shadowing_;
	};

//======================================================================
} // namespace plumbing
} // namespace dusk
//======================================================================
#endif
//======================================================================
