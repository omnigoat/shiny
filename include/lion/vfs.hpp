#pragma once

#include <lion/streams.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>
#include <atma/string.hpp>

#include <filesystem>


namespace stdfs = std::experimental::filesystem;

namespace lion
{
	enum class path_type_t
	{
		unknown,
		dir,
		file,
		symlink
	};

	struct fs_path_t;
	using  fs_path_ptr  = atma::intrusive_ptr<fs_path_t>;
	
	enum class open_flags_t
	{
		read,
		write,
		exclusive,
		nonbacked,
	};

	using open_mask_t = atma::bitmask_t<open_flags_t>;


	//
	// filesystem_t
	//
	struct filesystem_t : atma::ref_counted
	{
		virtual auto physical_path() const -> stdfs::path const& = 0;
		virtual auto root_path() const -> fs_path_ptr const& = 0;

		auto cd(fs_path_ptr const&, atma::string const&) -> fs_path_ptr const&;
		virtual auto cd(atma::string const& path) -> fs_path_ptr { return cd(root_path(), path); }
		virtual auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr { return stream_ptr::null; }

	protected:
		// subdirectory
		virtual auto impl_subpath(fs_path_ptr const&, char const*) -> fs_path_ptr { return fs_path_ptr::null; }

	private:
		friend struct fs_path_t;
	};

	using filesystem_ptr = atma::intrusive_ptr<filesystem_t>;

	inline auto split_path(atma::string const& str) -> atma::vector<atma::string>
	{
		char const* const delims = "\\/";

		atma::vector<atma::string> r;

		auto i = str.begin();
		while (i != str.end())
		{
			auto e = atma::find_first_of(str, i, delims);

			auto s = atma::string{i, e};
			s.push_back('/');
			r.push_back(s);
			i = ++e;
		}
		
		return r;
	}

	struct physical_filesystem_t : filesystem_t
	{
		physical_filesystem_t(stdfs::path const&);

		auto physical_path() const -> stdfs::path const& override { return physical_path_; }
		auto root_path() const -> fs_path_ptr const& override { return root_; }

		auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr override;

	private:
		auto impl_subpath(fs_path_ptr const&, char const*) -> fs_path_ptr override;

	private:
		stdfs::path physical_path_;
		fs_path_ptr root_;
	};

	using physical_filesystem_ptr = atma::intrusive_ptr<physical_filesystem_t>;

	struct fs_path_t : atma::ref_counted
	{
		using children_t = atma::vector<fs_path_ptr>;

		auto filesystem() const -> filesystem_ptr const& { return fs_; }
		auto parent() const -> fs_path_t* { return parent_; }
		auto children() const -> children_t const& { return children_; }
		auto path_type() const -> path_type_t { return type_; }
		auto leaf() const -> atma::string const& { return leaf_; }

		auto set_filesystem(filesystem_ptr const& fs) -> void { fs_ = fs; }

	private:
		fs_path_t(filesystem_ptr const&, fs_path_t* parent, path_type_t, stdfs::path const& logical, stdfs::path const& physical);

	private:
		filesystem_ptr fs_;
		fs_path_t* parent_;
		children_t children_;

		path_type_t type_;
		atma::string leaf_;

		friend struct filesystem_t;
		friend struct atma::intrusive_ptr_expose_constructor;
	};

	inline auto operator == (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		//return lhs.to_string() == rhs.to_string();
		return true;
	}

	inline auto operator != (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	
	inline auto remove_toplevel(stdfs::path const& p) -> stdfs::path
	{
		auto const& k = p.native();
		auto i = k.find(stdfs::path::preferred_separator);

		if (i == k.npos)
			return stdfs::path{};
		else
			return stdfs::path{k.c_str() + i + 1, k.c_str() + k.size()};
	}
	



	inline auto findinc_path_separator(char const* begin, char const* end) -> char const*
	{
		auto i = begin;
		while (i != end && *i != '/')
			++i;

		return (i == end) ? i : i + 1;
	}

	struct path_range_iter_t
	{
		path_range_iter_t(char const* begin, char const* end, char const* terminal)
			:range_{begin, end}, terminal_(terminal)
		{}

		auto operator * () const -> atma::utf8_string_range_t const&
		{
			return range_;
		}

		auto operator -> () const -> atma::utf8_string_range_t const*
		{
			return &range_;
		}

		auto operator ++ () -> path_range_iter_t&
		{
			range_ = atma::utf8_string_range_t{range_.end(), findinc_path_separator(range_.end(), terminal_)};
			return *this;
		}

		auto operator != (path_range_iter_t const& rhs) const -> bool
		{
			return range_ != rhs.range_ || terminal_ != rhs.terminal_;
		}

	private:
		atma::utf8_string_range_t range_;
		char const* terminal_;
	};

	struct path_range_t
	{
		path_range_t(char const* str, size_t size)
			: str_(str), size_(size)
		{}

		auto begin() -> path_range_iter_t
		{
			auto r = findinc_path_separator(str_, str_ + size_);
			return{str_, r, str_ + size_};
		}

		auto end() -> path_range_iter_t
		{
			return{str_ + size_, str_ + size_, str_ + size_};
		}

	private:
		char const* str_;
		size_t size_;
	};


	inline auto path_split_range(atma::string const& p) -> path_range_t
	{
		return{p.c_str(), p.raw_size()};
	}
	
	// 
	struct vfs_t
	{
		vfs_t();
		//vfs_t(atma::string const&);

		//auto root_path() const -> fs_path_ptr const& { return root_; }
		//auto physical_path() const -> stdfs::path const& { return root_->physical_path(); }

		auto mount(atma::string const&, filesystem_ptr const&) -> void;

		auto open(atma::string const&) -> stream_ptr;
		auto open(atma::string const&, open_mask_t) -> stream_ptr;

	private:
		struct mount_node_t
		{
			using children_t = atma::vector<mount_node_t>;

			atma::string name;
			filesystem_ptr filesystem;
			children_t children;
		};

	private:
		mount_node_t root_;
	};
}
