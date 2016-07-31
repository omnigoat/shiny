#pragma once

#include <lion/filesystem.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/vector.hpp>
#include <atma/atomic.hpp>

#include <tuple>


namespace lion
{
	struct asset_t;
	struct asset_id_t;
	struct asset_handle_t;
	struct asset_library_t;

	struct asset_t : atma::ref_counted
	{
		auto path() const -> path_t const& { return path_; }

	private:
		path_t path_;
	};

	using asset_ptr = atma::intrusive_ptr<asset_t>;

	struct asset_id_t
	{
		asset_id_t()
			: type(), generation(), unused(), idx()
		{}

		uint64 type : 8; // 256 different types of assets... enough?
		uint64 generation : 8; // 256 generations... enough? 255 if 0x0 means "always latest"
		uint64 unused : 16;
		uint64 idx : 32; // 32-bit number of this type. totes enough.
	};


	struct asset_handle_t
	{
		auto lock() -> asset_ptr; //{ return library_.lookup(id_); }
	private:
		asset_library_t* library_;
		asset_id_t id_;
	};
	
	struct asset_library_t
	{
		asset_library_t();

		auto gen_random() -> std::tuple<uint32, uint32>;
		auto release(std::tuple<uint32, uint32> const&) -> void;

		auto dump_ascii() -> void;

	private: // table management
		struct slot_t;
		struct freelist_node_t;
		struct freelist_head_t;
		struct page_t;

		//static uint32 const page_size;

		auto get_slot() -> std::tuple<page_t*, uint32>;

		static const uint32 pages_capacity_ = 256;
		uint32 pages_size_ = 0;

		uint32 page_slot_capacity_ = 128;

		page_t* pages_[pages_capacity_] = {};
		page_t* first_page_ = nullptr;

	private:
		vfs_t* vfs_;
		//asset_type_list_t types_;
	};

	struct asset_library_t::slot_t
	{
		asset_t* asset = nullptr;
		uint32 ref_count = 0;
		asset_id_t prev;
		std::atomic<bool> used;
	};

	struct asset_library_t::freelist_node_t
	{
		freelist_node_t()
			: next{nullptr}
		{}

		freelist_node_t(uint32 idx, freelist_node_t* n)
			: idx(idx), next(n)
		{}

		std::atomic<freelist_node_t*> next;
		uint32 idx = 0;
	};

	struct alignas(16) asset_library_t::freelist_head_t
	{
		freelist_head_t()
		{}

		freelist_head_t(freelist_node_t* n, uint32 aba)
			: node(n), aba(aba)
		{}

		freelist_node_t* node = nullptr;
		uint32 aba = 0;
	};

	struct asset_library_t::page_t
	{
		page_t(uint32 id, uint32 size)
			: id(id)
			, capacity{size}
			, memory{size}
			, freeslots{new uint32[std::max(size / 32, 1u)]()}
		{}

		uint32 id = 0;

		uint32 capacity = 0;
		uint32 size = 0;
		atma::memory_t<slot_t> memory;
		
		freelist_head_t freelist;
		uint32* freeslots = nullptr;
		
		alignas(8) page_t* next = nullptr;
	};

}
