#pragma once

#include <lion/filesystem.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/vector.hpp>


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
		auto gen_random() -> asset_handle_t;

	private:
		struct asset_storage_t;
		struct asset_type_storage_t;

		using asset_storage_list_t = atma::vector<asset_storage_t>;
		using asset_type_list_t = atma::vector<asset_storage_list_t>;

	private:
		vfs_t* vfs_;
		asset_type_list_t types_;
	};

	struct asset_library_t::asset_storage_t
	{
		asset_t* asset = nullptr;
		uint32 ref_count = 0;
		asset_storage_t* prev = nullptr;
	};
}
