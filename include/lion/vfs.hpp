#pragma once

#include <atma/intrusive_ptr.hpp>


namespace lion
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr  = atma::intrusive_ptr<path_t>;
	using  path_wptr = path_t*;

	// path
	struct path_t : atma::ref_counted
	{
		path_t();
		path_t(atma::string const&);

		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(atma::string const&, atma::string::const_iterator const&);

	private:
		using children_t = atma::vector<path_ptr>;

		atma::string name_;
		path_type_t type_;
		
		path_wptr parent_;
		children_t children_;
	};



	char const* const delims = "\\/";

	inline auto split_path(atma::string const& str) -> void
	{
	}

	path_t::path_t()
	{
	}

	path_t::path_t(atma::string const& str)
		//: path_t(str, str.end())
	{
		atma::vector<path_ptr> ancestors;


		auto i = str.begin();

		for (;;)
		{
			auto j = atma::find_first_of(i, str.end(), delims);
			if (j == i)
			{
				
			}
		}
	}

	path_t::path_t(atma::string const& str, atma::string::const_iterator const& begin)
	{
#if 0
		ATMA_ASSERT(end != str.begin());

		char const* delims = "/\\";

		auto self_begin = atma::find_first_of(str, begin, delims);
		if (self_begin == str.end()) {
			type_ = path_type_t::file;
		}
		else {
			type_ = path_type_t::dir;
			++end;
		}

		name_ = atma::string(begin, end);

		if (end == begin)
			return;

		child_ = path_ptr(new path_t(str, end));
#endif
	}


	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		//for (auto t = this; t != nullptr; t = t->child_.get())
		//{
		//	result += t->name_;
		//}

		return result;
	}

	inline auto operator == (path_t const& lhs, path_t const& rhs) -> bool
	{
		return lhs.to_string() == rhs.to_string();
	}

	inline auto operator != (path_t const& lhs, path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	struct abstract_filesystem_t
	{
		virtual auto generate_path(atma::string const&) -> path_ptr const& = 0;
	};

	struct physical_filesystem_t : abstract_filesystem_t
	{
		auto generate_path(atma::string const& p) -> path_ptr const& override
		{
			//auto things = atma::split_on_any(p, "/\\");
		}

	private:
		path_ptr root_;
	};



	// 
	struct vfs_t
	{
	};
}
