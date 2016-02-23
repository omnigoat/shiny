#include <sandbox/sandbox.hpp>
#include <sandbox/voxelization.hpp>

#include <shiny/runtime.hpp>
#include <shiny/context.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/data_declaration.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/camera.hpp>
#include <shiny/scene.hpp>
#include <shiny/texture2d.hpp>
#include <shiny/compute_shader.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/generic_buffer.hpp>
#include <shiny/draw_target.hpp>

#include <lion/filesystem.hpp>

#include <pepper/freelook_camera_controller.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/events/mouse.hpp>
#include <fooey/event_handler.hpp>
#include <fooey/keys.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/intersection.hpp>

#include <atma/filesystem/file.hpp>
#include <atma/algorithm.hpp>
#include <atma/function.hpp>
#include <atma/console.hpp>
#include <atma/atomic.hpp>

#include <regex>
#include <atomic>

#include <io.h>
#include <fcntl.h>

using namespace sandbox;
using sandbox::application_t;

float application_t::cube_vertices[] =
{
	0.5f, 0.5f, 0.5f, 1.f,    1.f, 0.f, 0.f, 0.2f,
	0.5f, 0.5f, -0.5f, 1.f,   0.f, 1.f, 0.f, 0.2f,
	0.5f, -0.5f, 0.5f, 1.f,   0.f, 0.f, 1.f, 0.2f,
	0.5f, -0.5f, -0.5f, 1.f,  1.f, 1.f, 0.f, 0.2f,
	-0.5f, 0.5f, 0.5f, 1.f,   1.f, 0.f, 1.f, 0.2f,
	-0.5f, 0.5f, -0.5f, 1.f,  0.f, 1.f, 1.f, 0.2f,
	-0.5f, -0.5f, 0.5f, 1.f,  1.f, 1.f, 1.f, 0.2f,
	-0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 0.f, 0.2f,
};

uint16 application_t::cube_indices[] =
{
	4, 5, 7, 7, 6, 4, // -x plane
	0, 2, 3, 3, 1, 0, // +x plane
	2, 6, 7, 7, 3, 2, // -y plane
	0, 1, 5, 5, 4, 0, // +y plane
	5, 1, 3, 3, 7, 5, // -z plane
	6, 2, 0, 0, 4, 6, // +z plane
};

extern int function_main();

template <typename T>
struct indirect_ptr
{
	indirect_ptr(T* const& ptr)
		: ptr_(&ptr)
	{}

	auto operator -> () const -> T*
	{
		return *ptr_;
	}

private:
	T* const* ptr_;
};

namespace lion
{
	struct asset_t {};

	struct asset_storage_t
	{
		//asset_ptr current;
		asset_t* current;
		//asset_ptr next;
		std::atomic_int32_t current_use_count;
		//std::atomic_int32_t next_use_count;
		std::atomic<asset_t*> ptr;
	};
}

using asset_handle_t = intptr; //asset_storage_t const*;


#if 0
char buf[256];


struct stdio_catcher_t
{
	static int const bufsize = 4096;
	static int const stdin_idx = 0, stdout_idx = 1, stderr_idx = 2, stdmax = 3;
	static int const fd_read = 0, fd_write = 1;

	stdio_catcher_t()
		: stdio_filehandles_{
			_fileno(stdin),
			_fileno(stdout),
			_fileno(stderr)
		}
	{
		engine_.signal([&]
		{
			// pipe stdout, stdin, stderr to *new places*
			//ATMA_ENSURE_IS(0, _pipe(fds_[0], 4096, _O_BINARY));
			ATMA_ENSURE_IS(0, _pipe(fds_[stdout_idx], 4096, _O_BINARY));
			ATMA_ENSURE_IS(0, _pipe(fds_[stderr_idx], 4096, _O_BINARY));

			for (int i = stdout_idx; i != stdmax; ++i)
			{
				int r = _dup2(fds_[i][fd_write], stdio_filehandles_[i]);
				ATMA_ASSERT(r != -1);
			}
		});

		engine_.signal_evergreen([&]{
			// do stderr first, because in the case of horrifying errors, we want
			// to tell the world as fast as possible
			int sz = _read(fds_[stderr_idx][fd_read], buf_[stderr_idx], sizeof(buf_[stderr_idx]) - 1);

			//int sz = _read(fds_[0][0], buf_[0], bufsize);
			//fwrite(buf_[0], 1, sz, stderr);
		});
	}

private:
	int stdio_filehandles_[3];
	int fds_[3][2]; // stdout, stdin, stderr {read|write}
	char buf_[3][bufsize];

	atma::thread::engine_t engine_;
};


enum class log_levels_t : int
{
	fatal = 0,
	error = 1,
	warning = 2,
	info = 3,
	trace = 4,

	size = 5
};

#endif



struct mwsr_queue_t
{
	struct allocation_t;
	struct decoder_t;

	mwsr_queue_t(void*, uint32);
	mwsr_queue_t(uint32);

	auto allocate(uint16 size, byte header) -> allocation_t;
	auto commit(allocation_t&) -> void;

	auto consume(decoder_t&) -> bool;

private:
	enum class command_t : byte;

	// header is {2-bytes: size, 2-bytes: id}
	static uint32 const header_szfield_size = 2;
	static uint32 const header_idfield_size = 2;
	static uint32 const header_size = header_szfield_size + header_idfield_size;

	auto buf_encode_header(byte*, uint32 bufsize, allocation_t&, command_t, byte = 0) -> void;
	auto buf_encode_byte  (byte*, uint32 bufsize, allocation_t&, byte) -> void;
	auto buf_encode_uint16(byte*, uint32 bufsize, allocation_t&, uint16) -> void;
	auto buf_encode_uint32(byte*, uint32 bufsize, allocation_t&, uint32) -> void;
	auto buf_encode_uint64(byte*, uint32 bufsize, allocation_t&, uint64) -> void;

	auto starve_flag(size_t starve_id, size_t thread_id, std::chrono::microseconds const& starve_time) -> void;
	auto starve_unflag(size_t starve_id, size_t thread_id) -> void;
	auto starve_gate(size_t thread_id) -> size_t;

private:
	bool owner_ = false;

	union
	{
		struct
		{
			byte*  write_buf_;
			uint32 write_buf_size_;
			uint32 write_position_;
		};

		atma::atomic128_t write_info_;
	};

	union
	{
		struct
		{
			byte* read_buf_;
			uint32 read_buf_size_;
			uint32 read_position_;
		};

		atma::atomic128_t read_info_;
	};

	union
	{
		struct
		{
			uint64 starve_thread_;
			uint64 starve_time_;
		};

		atma::atomic128_t starve_info_;
	};

	std::chrono::microseconds const starve_timeout{100};
};

struct mwsr_queue_t::allocation_t
{
	auto encode_byte  (byte) -> void;
	auto encode_uint16(uint16) -> void;
	auto encode_uint32(uint32) -> void;
	auto encode_uint64(uint64) -> void;

private:
	allocation_t(byte* buf, uint32 bufsize, uint32 wp, uint16 size)
		: buf(buf), bufsize(bufsize)
		, wp(wp), p(wp + header_szfield_size)
		, size(size)
	{
		ATMA_ASSERT(size > header_size, "bad command size");
	}

	byte*  buf;
	uint32 bufsize;
	uint32 wp, p;
	uint16 size;

	friend struct mwsr_queue_t;
};

struct mwsr_queue_t::decoder_t
{
	decoder_t() {}

	auto id() const -> byte { return id_; }

	auto decode_byte(byte&) -> void;
	auto decode_uint16(uint16&) -> void;
	auto decode_uint32(uint32&) -> void;
	auto decode_uint64(uint64&) -> void;

private:
	decoder_t(byte* buf, uint32 bufsize, uint32 rp)
		: buf(buf), bufsize(bufsize)
		, p(rp + header_szfield_size)
		, size(*(uint16*)(buf + rp))
	{
	}

	byte*  buf;
	uint32 bufsize;
	uint32 p;
	uint16 size;
	byte id_ = 0;

	friend struct mwsr_queue_t;
};

enum class mwsr_queue_t::command_t : byte
{
	nop,
	jump,
	user
};

mwsr_queue_t::mwsr_queue_t(void* buf, uint32 size)
	: write_buf_((byte*)buf)
	, write_buf_size_(size)
	, write_position_()
	, read_buf_((byte*)buf)
	, read_buf_size_(size)
	, read_position_()
	, starve_thread_()
	, starve_time_()
{}

mwsr_queue_t::mwsr_queue_t(uint32 sz)
	: owner_(true)
	, write_buf_(new byte[sz]{})
	, write_buf_size_(sz)
	, write_position_()
	, read_buf_(write_buf_)
	, read_buf_size_(write_buf_size_)
	, read_position_()
	, starve_thread_()
	, starve_time_()
{}

auto mwsr_queue_t::allocate(uint16 size, byte user_header) -> allocation_t
{
	// also need space for alloc header
	size += header_size;

	ATMA_ASSERT(size <= write_buf_size_, "queue can not allocate that much");

	size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());


	byte* wb = nullptr;
	uint32 wbs = 0;
	uint32 wp = 0;

	std::chrono::microseconds starvation{};
	for (;;)
	{
		auto time_start = std::chrono::high_resolution_clock::now();

		// starve_id is either zero or our thread-id, at any point
		// during this function-call "starve_.thread" could change
		auto starve_id = starve_gate(thread_id);

		// read 16 bytes of {write-buf, buf-size, write-pos}
		atma::atomic128_t q_write_info;
		atma::atomic_read128(&q_write_info, &write_info_);

		// write-buffer, write-buffer-size, write-position
		wb  = (byte*)q_write_info.ui64[0];
		wbs = q_write_info.ui32[2];
		wp  = q_write_info.ui32[3];

		// read-buffer, read-position
		atma::atomic128_t q_read_info;
		atma::atomic_read128(&q_read_info, &read_info_);
		auto rb  = (byte*)q_read_info.ui64[0];
		auto rp  = q_read_info.ui32[3];

		// size of available bytes. subtract one because we must never have
		// the write-position and read-position equal the same value if the
		// buffer is full, because we can't distinguish it from being empty
		//
		// if our buffers are differing (because we are mid-rebase), then
		// we can only allocate up to the end of the new buffer
		uint32 available = 0;
		if (wb == rb)
			available = (rp <= wp ? rp + wbs - wp : rp - wp) - 1;
		else
			available = wbs - wp - 1;


		// new-write-position can't be "one before" the end of the buffer,
		// as we must write the first two bytes of the header atomically
		auto nwp = (wp + size) % wbs;
		if (nwp == wbs - 1) {
			++nwp;
			++size;
		}

		if (size <= available)
		{
			if (atma::atomic_compare_exchange(&write_position_, wp, nwp))
			{
				starve_unflag(starve_id, thread_id);
				break;
			}
		}
		// not enough space for our command. we may eventually have to block (busy-wait),
		// but first let's try to create a *new* buffer, atomically set all new writes to
		// write into that buffer, and encode a jump command in our current buffer. jump
		// commands are very small (16 bytes), so maybe that'll work.
		else
		{
			static_assert(sizeof(uint64) >= sizeof(void*), "pointers too large! where are you?");

			// new write-information
			atma::atomic128_t nwi;
			auto const nwbs = wbs * 2;
			nwi.ui64[0] = (uint64)new byte[nwbs]();
			nwi.ui32[2] = nwbs;
			nwi.ui32[3] = 0;


			uint16 const growcmd_size = 12 + 4;
			if (growcmd_size <= available)
			{
				uint32 gnwp = (wp + growcmd_size) % wbs;

				if (atma::atomic_compare_exchange(&write_position_, wp, gnwp))
				{
					auto A = allocation_t{wb, wbs, wp, growcmd_size};

					// update "known" write-position
					q_write_info.ui32[3] = gnwp;

					if (atma::atomic_compare_exchange(&write_info_, q_write_info, nwi))
					{
						// successfully (atomically) moved to new buffer
						// encode "jump" command to new buffer
						A.encode_byte((byte)command_t::jump);
						A.encode_byte(0);
						A.encode_uint64(nwi.ui64[0]);
						A.encode_uint32(nwbs);
					}
					else
					{
						// we failed to move to new buffer: encode a nop
						buf_encode_header(wb, wbs, A, command_t::nop);
						delete [] (byte*)nwi.ui64[0];
					}

					commit(A);
				}
			}
			else
			{
				delete [] (byte*)nwi.ui64[0];
			}
		}

		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time_start);
		starvation += elapsed;

		starve_unflag(starve_id, thread_id);
		starve_flag(starve_id, thread_id, starvation);
	}

	auto A = allocation_t{wb, wbs, wp, size};
	A.encode_byte((byte)command_t::user);
	A.encode_byte(user_header);
	return A;
}

auto mwsr_queue_t::commit(allocation_t& a) -> void
{
	// we have already guaranteed that the first two bytes of the header
	// does not wrap around our buffer
	atma::atomic_exchange(a.buf + a.wp, a.size);
	a.wp = a.p = a.size = 0;
}

auto mwsr_queue_t::buf_encode_header(byte* buf, uint32 bufsize, allocation_t& A, command_t c, byte b) -> void
{
	buf_encode_byte(buf, bufsize, A, (byte)c);
	buf_encode_byte(buf, bufsize, A, b);
}

auto mwsr_queue_t::buf_encode_byte(byte* buf, uint32 bufsize, allocation_t& A, byte b) -> void
{
	ATMA_ASSERT(A.p != (A.wp + A.size) % bufsize);
	ATMA_ASSERT(0 <= A.p && A.p < bufsize);

	buf[A.p] = b;
	A.p = (A.p + 1) % bufsize;
}

auto mwsr_queue_t::buf_encode_uint16(byte* buf, uint32 bufsize, allocation_t& A, uint16 i) -> void
{
	buf_encode_byte(buf, bufsize, A, i & 0xff);
	buf_encode_byte(buf, bufsize, A, (i & 0xff00) >> 8);
}

auto mwsr_queue_t::buf_encode_uint32(byte* buf, uint32 bufsize, allocation_t& A, uint32 i) -> void
{
	buf_encode_uint16(buf, bufsize, A, i & 0xffff);
	buf_encode_uint16(buf, bufsize, A, (i & 0xffff0000) >> 16);
}

auto mwsr_queue_t::buf_encode_uint64(byte* buf, uint32 bufsize, allocation_t& A, uint64 i) -> void
{
	buf_encode_uint32(buf, bufsize, A, i & 0xffffffff);
	buf_encode_uint32(buf, bufsize, A, i >> 32);
}


auto mwsr_queue_t::starve_gate(size_t thread_id) -> size_t
{
	size_t st = 0;
	do {
		st = starve_thread_;
	} while (st != 0 && st != thread_id);

	return st;
}

auto mwsr_queue_t::starve_flag(size_t starve_id, size_t thread_id, std::chrono::microseconds const& starve_time) -> void
{
	if (starve_time > starve_timeout)
	{
		atma::atomic128_t q;
		atma::atomic_read128(&q, &starve_info_);
		auto q_starve_thread = q.ui64[0];
		auto q_starve_time = q.ui64[1];

		// we can update the starvation information if we're just updating ourselves, or
		// we've been starved longer than what's already seen
		if (q_starve_thread == thread_id || starve_time.count() > q_starve_time)
		{
			atma::atomic128_t v{thread_id, (uint64)starve_time.count()};

			// this only tries once!
			atma::atomic_compare_exchange(&starve_info_, q, v);
		}
	}
}

auto mwsr_queue_t::starve_unflag(size_t starve_id, size_t thread_id) -> void
{
	atma::atomic_compare_exchange(&starve_thread_, (uint64)thread_id, uint64());

	//if (starve_.thread == thread_id)
	//{
	//	ATMA_ENSURE(atma::atomic_compare_exchange(&starve_.thread, (uint64)thread_id, uint64()), "shouldn't have contention over resetting starvation");
	//}
}

auto mwsr_queue_t::consume(decoder_t& D) -> bool
{
	// read first 16 bytes of header
	uint16 size = *(uint16*)(read_buf_ + read_position_);
	if (size == 0)
		return false;

	D.buf = read_buf_;
	D.bufsize = read_buf_size_;
	D.p = read_position_ + 2;
	D.size = size;

	// read second 16 bytes of header
	byte header;
	D.decode_byte(header);
	D.decode_byte(D.id_);

	read_position_ = (read_position_ + size) % read_buf_size_;

	switch ((command_t)header)
	{
		// nop means we move to the next location
		case command_t::nop:
			return consume(D);

		// jump to the encoded, larger, read-buffer
		case command_t::jump:
		{
			uint64 ptr;
			uint32 size;
			D.decode_uint64(ptr);
			D.decode_uint32(size);
			delete [] read_buf_;
			read_buf_ = (byte*)ptr;
			read_buf_size_ = size;
			read_position_ = 0;
			std::cout << "JUMPED TO SIZE " << size << std::endl;
			return consume(D);
		}

		// user
		default:
			break;
	}

	return true;
}

auto mwsr_queue_t::allocation_t::encode_byte(byte b) -> void
{
	ATMA_ASSERT(p != (wp + size) % bufsize);
	ATMA_ASSERT(0 <= p && p < bufsize);

	buf[p] = b;
	p = (p + 1) % bufsize;
}

auto mwsr_queue_t::allocation_t::encode_uint16(uint16 i) -> void
{
	encode_byte(i & 0xff);
	encode_byte(i >> 8);
}

auto mwsr_queue_t::allocation_t::encode_uint32(uint32 i) -> void
{
	encode_uint16(i & 0xffff);
	encode_uint16(i >> 16);
}

auto mwsr_queue_t::allocation_t::encode_uint64(uint64 i) -> void
{
	encode_uint32(i & 0xffffffff);
	encode_uint32(i >> 32);
}

auto mwsr_queue_t::decoder_t::decode_byte(byte& b) -> void
{
	b = buf[p];
	p = (p + 1) % bufsize;
}

auto mwsr_queue_t::decoder_t::decode_uint16(uint16& i) -> void
{
	byte* bs = (byte*)&i;

	decode_byte(bs[0]);
	decode_byte(bs[1]);
}

auto mwsr_queue_t::decoder_t::decode_uint32(uint32& i) -> void
{
	byte* bs = (byte*)&i;

	decode_byte(bs[0]);
	decode_byte(bs[1]);
	decode_byte(bs[2]);
	decode_byte(bs[3]);
}

auto mwsr_queue_t::decoder_t::decode_uint64(uint64& i) -> void
{
	byte* bs = (byte*)&i;

	decode_byte(bs[0]);
	decode_byte(bs[1]);
	decode_byte(bs[2]);
	decode_byte(bs[3]);
	decode_byte(bs[4]);
	decode_byte(bs[5]);
	decode_byte(bs[6]);
	decode_byte(bs[7]);
}


#if 0

struct log_system_t
{
	static size_t const buf_size = 76;
	static size_t const thread_buf_size = 2048;
	
	log_system_t()
	{
		memset(buf_, 0, buf_size);

		handle_ = std::thread([&] {
			while (running_)
			{
				while (read_position_ != written_position_)
				{
					decode_command();
				}
			}
		});
	}

	~log_system_t()
	{
		running_ = false;
		handle_.join();
	}

	template <typename... Args>
	auto signal_log(Args&&... args) -> void
	{
		// +1 size for newline appended to all logging
		size_t reqsize = 0;
		acc_size(reqsize, "\n");
		ATMA_SPLAT_FN(acc_size(reqsize, args));

		auto A = buf_allocate(reqsize);
		ATMA_SPLAT_FN(encode(A, args));
		encode(A, "\n");

		buf_commit(A);
	}

	auto signal_test() -> void
	{
		auto A = buf_allocate(39);

		char tidbuf[36];
		auto n = sprintf(tidbuf, "thread %hu [wp %zu, nwp %zu]\n\0", (uint16)std::hash<std::thread::id>{}(std::this_thread::get_id()), A.wp, A.nwp);
		//printf(tidbuf);

		encode_byte   (A, (byte)identifier_t::string);
		encode_uint16 (A, uint16(n));
		encode_mem    (A, tidbuf, n);

		if (n < 33)
			encode_byte(A, 0);

		//char membuf[24];
		//auto n2 = sprintf(membuf, "u", );

		//encode_byte   (A, (byte)identifier_t::string);
		//encode_uint16 (A, uint16(n));
		//encode_mem    (A, membuf, n);
		buf_commit(A);
	}

private:
	auto acc_size(size_t& s, char const* str) -> void { s += 3 + strlen(str); }
	auto acc_size(size_t& s, byte) -> void { s += 2; }

private:
	//--------------------------------
	//  starvation prevention
	//--------------------------------
	auto strv_gate(size_t thread_id) -> size_t
	{
		size_t st = 0;
		do {
			st = starve_.thread;
		} while (st != 0 && st != thread_id);

		return st;
	}

	auto strv_flag(size_t starve_id, size_t thread_id, std::chrono::microseconds const& starve_time) -> void
	{
		if (starve_time > starve_timeout)
		{
			auto q_starve_thread = starve_.thread;
			auto q_starve_time = starve_.time;

			// if we've been starved longer than anything else, we're first!
			if (starve_time.count() > q_starve_time)
			{
				size_t exp[] = {q_starve_thread, q_starve_time};

				uint16 k;
				atma::atomic_exchange<uint16>(&k, 4);

				// this only tries once!
				InterlockedCompareExchange128((LONG64*)&starve_, thread_id, starve_time.count(), (LONG64*)exp);
			}
			else
			// if we're the starving thread
			if (q_starve_thread == thread_id)
			{
				starve_.time = starve_time;
			}
			else
			{
				//
				//uint64 exp = 0;
				//while (!starve_.thread.compare_exchange_strong(exp, thread_id))
					//exp = 0;
			}

			stave_time_ = starve_time;
		}
	}

	auto strv_unflag(size_t starve_id, size_t thread_id) -> void
	{
		if (starve_.thread == thread_id)
		{
			auto exp = thread_id;
			auto r = starve_.thread.compare_exchange_strong(exp, 0);
			ATMA_ASSERT(r, "shouldn't have contention over resetting starvation");
		}
	}

private:
	//--------------------------------
	//  buffer mutations
	//--------------------------------
	static int const header_size = sizeof(size_t);

	struct buf_allocation_t
	{
		size_t wp;
		size_t nwp;
		size_t p;
	};
	
	auto buf_allocate(size_t size) -> buf_allocation_t
	{
		// also need space for alloc header
		size += 2;

		ATMA_ASSERT(size <= buf_size, "queue can not allocate that much");

		// write-position, read-position, new-write-position
		size_t wp = 0, rp = 0, nwp = 0;

		size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());

		std::chrono::microseconds starvation{};
		for (;;)
		{
			auto time_start = std::chrono::high_resolution_clock::now();

			auto starve_id = strv_gate(thread_id);

			wp = write_position_.load();
			rp = read_position_.load();

			// size of available bytes. subtract one because we must never have
			// the write-position and read-position equal the same value if the
			// buffer is full, because we can't distinguish it from being empty
			auto available = (rp <= wp ? rp + buf_size - wp : rp - wp) - 1;

			if (size <= available)
			{
				nwp = (wp + size) % buf_size;

				if (write_position_.compare_exchange_strong(wp, nwp))
				{
					strv_unflag(starve_id, thread_id);
					break;
				}
			}
			
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time_start);
			starvation += elapsed;

			strv_flag(starve_id, thread_id, starvation);
		}

		// encode chunk header
		auto A = buf_allocation_t{wp, nwp, wp};
		encode_uint16(A, (uint16)size);

		return A;
	}

	auto buf_commit(buf_allocation_t const& a) -> void
	{
		auto exp = a.wp;
		while (!written_position_.compare_exchange_strong(exp, a.nwp))
			exp = a.wp;
	}

private:
	//--------------------------------
	//  encoding
	//--------------------------------
	enum class identifier_t : byte
	{
		pad,
		string,
		color,
	};

	auto encode_byte(buf_allocation_t& A, byte b) -> void
	{
		ATMA_ASSERT(A.p != A.nwp);

		buf_[A.p % buf_size] = b;
		A.p = (A.p + 1) % buf_size;
	}

	auto encode_uint16(buf_allocation_t& A, uint16 i) -> void
	{
		encode_byte(A, i & 0xff);
		encode_byte(A, (i & 0xff00) >> 8);
	}

	auto encode_mem(buf_allocation_t& A, void const* buf, size_t size) -> void
	{
		for (size_t i = 0; i != size; ++i) {
			encode_byte(A, ((byte*)buf)[i]);
		}
	}

	auto encode(buf_allocation_t& A, byte color) -> void
	{
		encode_byte(A, (byte)identifier_t::color);
		encode_byte(A, color);
	}

	auto encode(buf_allocation_t& A, char const* string) -> void
	{
		size_t size = strlen(string);

		encode_byte(A, (byte)identifier_t::string);
		encode_uint16(A, uint16(size));
		encode_mem(A, string, size);
	}

	//--------------------------------
	//  decoding
	//--------------------------------
	auto decode_byte(byte& x, size_t p) -> size_t 
	{
		x = buf_[p];
		return (p + 1) % buf_size;
	}

	auto decode_uint16(uint16& x, size_t p) -> size_t
	{
		byte h, l;
		p = decode_byte(l, p);
		p = decode_byte(h, p);
		x = (uint16(h) << 8) | l;
		return p;
	}

	auto decode_string(atma::string& x, size_t p, size_t s) -> size_t
	{
		for (size_t i = 0; i != s; ++i)
			x.push_back(buf_[(p + i) % buf_size]);

		return (p + s) % buf_size;
	}

	auto decode_command() -> void
	{
		auto rp = read_position_.load();
		auto p = rp;

		uint16 chunk_size;
		p = decode_uint16(chunk_size, rp);

		uint16 cp = 2;
		while (cp != chunk_size)
		{
			ATMA_ASSERT(cp < chunk_size, "we overran our read-buffer");

			byte identifier;
			p = decode_byte(identifier, p);
			++cp;

			switch (identifier)
			{
				case identifier_t::pad:
				{
					p += (chunk_size - cp);
					cp = chunk_size;
					break;
				}

				case identifier_t::string:
				{
					uint16 size;
					atma::string str;

					p = decode_uint16(size, p);
					p = decode_string(str, p, size);
					cp += 2 + size;

					std::cout << str;

					break;
				}

				case identifier_t::color:
				{
					byte color;

					p = decode_byte(color, p);
					++cp;

					atma::console::set_std_out_color(atma::console::combined_color_t{color});

					break;
				}

				default:
					ATMA_HALT("bad~~");
					break;
			}
		}

		read_position_ = (rp + chunk_size) % buf_size;
	}

private:
	atma::vector<void*> writers_;

	std::chrono::microseconds const starve_timeout{100};

	std::thread handle_;
	bool running_ = true;

	//byte buf_[buf_size];
	byte* write_buf_;
	std::atomic_size_t write_position_ = 0;

	byte* read_buf_;
	std::atomic_size_t written_position_ = 0;
	std::atomic_size_t read_position_ = 0;

	struct alignas(16) {
		size_t thread = 0;
		size_t time = 0;
	} starve_;
};
#endif


static int plus(int a, int b) { return a + b; }

application_t::application_t()
	: window_renderer(fooey::system_renderer())
	, window(fooey::window("Excitement!", 800 + 16, 600 + 38))
	, runtime{}
{
	//auto r1 = std::atomic_uint64_t::is_lock_free();
	//InterlockedCompareExchange128();

 {
	#if 0
	log_system_t shiny_log_system{
		//atma::log_color_t{log_system_t::fatal, 0b11001111},
		//atma::log_color_t{log_system_t::error, 0b11001111},
		//atma::log_color_t{log_system_t::warning, 0b00001110},
	};
	#endif

	mwsr_queue_t q{1024};
	
	auto rt = std::thread([&] {
		mwsr_queue_t::decoder_t D;

		for (;;) {
			if (q.consume(D)) {
				uint64 p;
				D.decode_uint64(p);
				std::cout << "thread id: " << std::hex << p << std::endl;
			}
		}
	});

	auto te = std::thread([&] {
		for (;;) {
			if (SHORT s = GetAsyncKeyState(VK_ESCAPE)) {
				if (s & 0x8000)
					exit(0);
			}
		}
	});

	auto t1 = std::thread([&] {
		for (;;) {
			auto A = q.allocate(8, 0);
			A.encode_uint64(std::hash<std::thread::id>{}(std::this_thread::get_id()));
			q.commit(A);
		}
	});

	auto t2 = std::thread([&] {
		for (;;) {
			auto A = q.allocate(80, 0);
			A.encode_uint64(std::hash<std::thread::id>{}(std::this_thread::get_id()));
			q.commit(A);
			Sleep(10);
		}
	});

#if 0
	auto t3 = std::thread([&] {
		for (;;)
			shiny_log_system.signal_log(0b00001100, "thread 3!");
		//shiny_log_system.signal_test();
	});

	auto t4 = std::thread([&] {
		for (;;)
			shiny_log_system.signal_log(0b00001111, "thread 4: here is a lot of text, trying to saturate the buffer..");
		//shiny_log_system.signal_test();
	});
#endif

	t1.join();
	t2.join();

	//shiny_log_system.signal_log(0b11110000, "here is a story about ", 0b00001100, "dragons.");
	//shiny_log_system.signal_log(0b00000111, "once upon a time, they were everywhere.");
	//shiny_log_system.signal_log("then they learnt how to brew gin.");
	//shiny_log_system.signal_log("so now they're mostly,");
	//shiny_log_system.signal_log("at the bar.");
	//shiny_log_system.signal_log("but they're also everywhere. alcohol helps with the procreating.");
 }
	//auto shiny_logpipe_handle = atma::log::new_pipe("shiny");

	exit(0);
#if 0
	int fds[2];
	
	int res = _pipe(fds, 4096, _O_BINARY);
	ATMA_ASSERT(res == 0);

	int so = _fileno(stdout);
	res = _dup2(fds[1], so);
	//ATMA_ASSERT(res != -1);
	
	char ttry[128];
	setvbuf(stdout, ttry, _IOLBF, 128);
	printf("blamalam\n");
	printf("yay\n");
	//fflush(stdout);
	
	res = _read(fds[0], buf, sizeof(buf) - 1);
#endif
	
	
	shiny::vertex_shader_t
	  * vs_basic = nullptr,
	  * vs_debug = nullptr,
	  * vs_voxel = nullptr;

	//LION_SCOPE_LOCK_ASSETS(
	//	(h1, vs_basic),
	//	(h2, vs_debug),
	//	(h2, vs_debug));

	lion::vfs_t vfs;
	auto fs = lion::physical_filesystem_ptr::make("./resources/published");
	vfs.mount("/res/", fs);

	auto f = vfs.open("/res/shaders/vs_basic.hlsl");
	auto m = lion::read_all(f);
	//std::cout << (char*)m.begin() << std::endl;
	printf("%.*s", (int)m.size(), m.begin());

	struct asset_pattern_t
	{
		std::regex regex;
		std::function<void()> callback;
	};

	//std::regex R{"^vs_.+\\.hlsl"};
	//bool b = std::regex_match("vs_love.hlsl", R);

	//char buf[8000];
	//auto r = f2->read(buf, 1200);
	
	//auto f = vfs.open("/res/shaders/vs_basic.hlsl", file_bind_flags::read_only);
#if 0
	lion::asset_library_t library{vfs};
	library.register_asset_thing("/res/shaders/",
		lion::open_flags_t::read,
		lion::file_watching_flags_t::yes,
		{ lion::asset_pattern{"vs_.+\\.hlsl", &load_vertex_shader},
		  lion::asset_pattern{"fs_.+\\.hlsl", &load_fragment_shader},
		  lion::asset_pattern{"cs_.+\\.hlsl", &load_compute_shader} });
		
	[](lion::input_stream_t const& stream) {
		
	});
#endif

	//library.register_asset_type("*\\.hlsl$", [](lion::input_stream_t const& stream) {
		// do things with f, return an asset_ptr
	//});

	//auto sh = library.load_asset_as<shiny::vertex_shader_t>("/res/shaders/);

	struct vertex_shader_backend_t
	{
		auto d3d_vs() const -> shiny::platform::d3d_vertex_shader_ptr { return {}; }
	};

	using vertex_shader_backend_ptr = indirect_ptr<vertex_shader_backend_t>;

	struct vertex_shader_tx
	{
		auto d3d_vs() const -> shiny::platform::d3d_vertex_shader_ptr
		{
			return backend_->d3d_vs();
		}

	private:
		//vertex_shader_backend_t const* const* backend_;
		vertex_shader_backend_ptr backend_;
	};

	//auto vs = lion::lock_asset_ptr(vertex_shader_handle);

	exit(0);

	function_main();

	window_renderer->add_window(window);
	ctx = shiny::create_context(runtime, window, shiny::primary_adapter);

	// geometry
	dd_position = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4}
	});

	dd_position_color = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4},
		{"color", 0, shiny::format_t::f32x4}
	});

	vb_cube = shiny::create_vertex_buffer(ctx, shiny::resource_storage_t::immutable, dd_position_color, 8, cube_vertices);
	ib_cube = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::format_t::u16, 36, cube_indices);


	// shaders
	auto vs_basic_file = atma::filesystem::file_t("../../shaders/vs_basic.hlsl");
	auto vs_basic_mem = vs_basic_file.read_into_memory();
	vs_flat = shiny::create_vertex_shader(ctx, dd_position, vs_basic_mem, false);

	auto ps_basic_file = atma::filesystem::file_t("../../shaders/ps_basic.hlsl");
	auto ps_basic_mem = ps_basic_file.read_into_memory();
	fs_flat = shiny::create_fragment_shader(ctx, ps_basic_mem, false);
}

auto application_t::run() -> int
{
	bool running = true;


	//
	//  generic input handling
	//
	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [&]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	// camera-controller
	auto&& cc = pepper::freelook_camera_controller_t{window};
	cc.require_mousedown_for_rotation(true);


	// clear!
	ctx->signal_draw_scene(shiny::scene_t{ctx, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}});


	//
	//  initialize plugins
	//
	for (auto const& x : plugins_)
	{
		x->input_bind(window);
		x->gfx_setup(ctx);
		x->main_setup();
	}


	// timestep of 16ms = 60hz
	auto const timestep_uint = 16u;
	auto const timestep = std::chrono::milliseconds(timestep_uint);
	
	// main-loop
	auto time_frame_end = std::chrono::high_resolution_clock::time_point();
	while (running)
	{
		auto time_now = std::chrono::high_resolution_clock::now();
		
		if (std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_frame_end) <= timestep)
			continue;
		time_frame_end = time_now;

		cc.update(timestep_uint);

		for (auto const& x : plugins_) {
			x->input_update();
		}

		ctx->immediate_set_stage(shiny::renderer_stage_t::resource_upload);

		// all plugins draw to same scene
		ctx->immediate_set_stage(shiny::renderer_stage_t::render);
#if 1
		auto&& scene = shiny::scene_t{ctx, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}};
		for (auto const& x : plugins_) {
			x->gfx_ctx_draw(ctx);
			x->gfx_draw(scene);
		}
		ctx->signal_draw_scene(scene);
#endif
		ctx->signal_present();
		ctx->signal_block();
	}

	return 0;
}


auto application_t::register_plugin(plugin_ptr const& plugin) -> void
{
	plugins_.push_back(plugin);
}

auto plugin_t::dd_position() const -> shiny::data_declaration_t const*
{
	return app_->dd_position;
}

auto plugin_t::dd_position_color() const -> shiny::data_declaration_t const*
{
	return app_->dd_position_color;
}

auto plugin_t::cube_vertices() const -> float const*
{
	return app_->cube_vertices;
}

auto plugin_t::cube_indices() const -> uint16 const*
{
	return app_->cube_indices;
}

auto plugin_t::vs_flat() const -> shiny::vertex_shader_ptr const&
{
	return app_->vs_flat;
}

auto plugin_t::fs_flat() const -> shiny::fragment_shader_ptr const&
{
	return app_->fs_flat;
}

int main()
{
	sandbox::application_t app;

	app.register_plugin(plugin_ptr(new sandbox::voxelization_plugin_t{&app}));

	return app.run();
}


