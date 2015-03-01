#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/math/intersection.hpp>
#include <atma/bind.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/filesystem/file.hpp>

#include <filesystem>

#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <shelf/file.hpp>
#include <sstream>



namespace shelf
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr = std::unique_ptr<path_t>; // atma::intrusive_ptr<path_t>;

	// path
	struct path_t
	{
		path_t();
		path_t(atma::string const&);
		path_t(path_t const&);

		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(atma::string const&, atma::string::const_iterator const&);

	private:
		atma::string name_;
		path_type_t type_;
		path_ptr child_;
	};


	path_t::path_t()
	{
	}

	path_t::path_t(atma::string const& str)
		: path_t(str, str.begin())
	{
	}

	path_t::path_t(atma::string const& str, atma::string::const_iterator const& begin)
	{
		char const* delims = "/\\";

		auto end = atma::find_first_of(str, begin, delims);
		if (end == str.end()) {
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
	}

	path_t::path_t(path_t const& rhs)
		: name_(rhs.name_), type_(rhs.type_)
	{
		if (rhs.child_)
			child_ = path_ptr(new path_t(*rhs.child_));
	}

	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		for (auto t = this; t != nullptr; t = t->child_.get())
		{
			result += t->name_;
		}

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


	struct filesystem_t
	{
		auto add_mount(path_t const& virtual_path, path_t const& host_path) -> void;

	private:
		struct node_t;
		using  node_ptr = atma::intrusive_ptr<node_t>;
		using  nodes_t = std::vector<node_ptr>;
		struct mount_t;

		struct node_t : atma::ref_counted
		{
			atma::string name;
			path_type_t type;

			node_t* parent;
			nodes_t children;
		};

		struct mount_t
		{
			path_t host_path;
			path_t mount_path;
			node_ptr mount_node;
		};


	private:
		auto merge_paths(node_t*, node_t*) -> node_ptr;

	private:
		typedef std::vector<mount_t> mounts_t;

		mounts_t mounts_;
	};

	auto filesystem_t::add_mount(path_t const& mount_path, path_t const& host_path) -> void
	{
#ifdef _DEBUG
		atma::for_each(mounts_, [&](mount_t const& x) {  ATMA_ASSERT(x.host_path != host_path);  });
#endif

		// do a thing?
		// host_path   = "relative/path/";
		// host_path2  = "/absolute/path";
		// host_path3  = "/absolute/different/path";

		mounts_.push_back({
			host_path,
			mount_path,
			// something??
		});
	}



	template <size_t Bufsize, typename FN>
	auto for_each_line(abstract_input_stream_t& stream, size_t maxsize, FN&& fn) -> void
	{
		char buf[Bufsize];
		atma::string line;

		// read bytes into line until newline is found
		auto rr = shelf::read_result_t{shelf::stream_status_t::ok, 0};
		while (rr.status == shelf::stream_status_t::ok)
		{
			rr = stream.read(buf, Bufsize);
			auto bufend = buf + rr.bytes_read;
			auto bufp = buf;

			while (bufp != bufend)
			{
				auto newline = std::find(bufp, bufend, '\n');
				line.append(bufp, newline);

				if (newline != bufend) {
					fn(line.raw_begin(), line.raw_size());
					line.clear();
				}

				bufp = (newline == bufend) ? bufend : newline + 1;
			}
		}
	}


}

namespace math = atma::math;


#include <atma/math/aabc.hpp>
#include <atma/unique_memory.hpp>
#include <array>


template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_allocate_tag
{
	explicit octree_allocate_tag(uint levels) : levels(levels) {}
	uint levels;
};

struct octree_t
{
	octree_t() {}

	octree_t(octree_allocate_tag t);

	auto insert(math::triangle_t const&) -> bool;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;

private:
	node_t* root_;

private:

private:
	
};

struct octree_t::node_t
{
	node_t()
		: view_(buf_)
	{}

	node_t(math::aabc_t const& aabc)
		: aabc(aabc), view_(buf_)
	{}

	node_t(octree_allocate_tag tag, math::aabc_t const& aabc)
		: aabc(aabc), view_(buf_)
	{
		imem_allocate(tag);
	}

	atma::math::aabc_t aabc;
	atma::math::triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::triangle_t const&) -> bool;

private:
	auto child_ref(uint idx) -> node_t& { return reinterpret_cast<node_t*>(buf_.begin())[idx]; }

	auto imem_allocate(octree_allocate_tag tag) -> void
	{
		if (tag.levels == 0)
			return;

		ATMA_ASSERT(buf_.empty());
		auto newbuf = atma::unique_memory_t{8 * sizeof(node_t)};
		std::swap(buf_, newbuf);

		auto subtag = octree_allocate_tag{tag.levels - 1};
		for (auto i = 0u; i != 8u; ++i)
			new (&child_ref(i)) node_t {subtag, aabc.octant_of(i)};
	}

#if 0
	auto oct_split() -> void
	{
		imem_allocate(octree_allocate_tag{1});

		for (auto const& x : data_)
			insert(x);
	}
#endif

	auto is_leaf() const -> bool { return buf_.empty(); }

	template <typename FN>
	auto imem_for_each(FN&& fn) -> void
	{
		for (auto i = buf_.begin<node_t>(); i != buf_.end<node_t>(); ++i)
			fn(*i);
		//for (auto& x : view_)
		//	fn(x);
	}

private:
	atma::unique_memory_t buf_;
	atma::memory_view_t<atma::unique_memory_t, node_t> view_;

	std::vector<math::triangle_t> data_;
};




octree_t::octree_t(octree_allocate_tag t)
	: root_( new node_t{t, math::aabc_t{}} )
{};

auto octree_t::insert(math::triangle_t const& tri) -> bool
{
	return root_->insert(tri);
}

auto octree_t::node_t::insert(math::triangle_t const& tri) -> bool
{
	if (!atma::math::intersect_aabc_triangle2(aabc, tri))
	{
		return false;
	}

	

	data_.push_back(tri);

	if (!buf_.empty())
		imem_for_each([&tri](node_t& x) {
			x.insert(tri);
		});

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return aabc.inside(point);
}




auto go() -> void
{
	auto oct = octree_t{octree_allocate_tag{4}};

	auto tri = math::triangle_t{
		math::point4f(0.1f, 0.1f, 0.1f),
		math::point4f(0.4f, 0.2f, 0.15f),
		math::point4f(0.23f, 0.56f, 0.44f)};

	auto tri2 = math::triangle_t {
		math::point4f(0.6f, 0.6f, 0.6f),
		math::point4f(0.6f, 0.2f, 0.1f),
		math::point4f(0.1f, 0.4f, 0.6f)};

	auto r = oct.insert(tri);
	auto r2 = oct.insert(tri2);
}


struct obj_model_t
{
	using verts_t = std::vector<math::vector4f>;
	using faces_t = std::vector<math::vector4i>;

	obj_model_t(shelf::file_t&);

	auto vertices() const -> verts_t const& { return verts_; }
	auto faces() const -> faces_t const& { return faces_; }

private:
	verts_t verts_;
	faces_t faces_;
};

struct mesh_t
{
	using vertices_t = std::vector<math::vector4f>;
	using indices_t = std::vector<math::vector4i>;

	mesh_t();
	mesh_t(size_t vertices, size_t indices);

private:
	
};

obj_model_t::obj_model_t(shelf::file_t& file)
{
	shelf::for_each_line<256>(file, 128, [&](char const* str, size_t size)
	{
		switch (str[0])
		{
			case 'v': {
				float x, y, z;
				sscanf(str, "v %f %f %f", &x, &y, &z);
				verts_.push_back(math::point4f(x, y, z));
				break;
			}

			case 'f': {
				int32 a, b, c;
				sscanf(str, "f %d %d %d", &a, &b, &c);
				faces_.push_back(math::vector4i{a, b, c, 0});
				break;
			}
		}
	});
}


int main()
{
	auto oct = octree_t{octree_allocate_tag{3}};
	oct.insert(math::triangle_t{math::point4f(0.1f, 0.1f, 0.1f), math::point4f(0.6f, 0.2f, 0.3f), math::point4f(-0.10f, 0.2f, 0.44f)});

#if 0
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	auto f2 = shelf::file_t{"../../data/dragon2.msh", shelf::file_access_t::write};

	uint64 verts = obj.vertices().size();
	uint64 faces = obj.faces().size();
	f2.write(&verts, sizeof(uint64));
	f2.write(&faces, sizeof(uint64));

	f2.write(&obj.vertices()[0], sizeof(math::vector4f) * verts);
	f2.write(&obj.faces()[0], sizeof(math::vector4i) * faces);
#elif 0
	auto msh_file = shelf::file_t {"../../data/dragon2.msh"};
	auto msh = msh_model_t{msh_file};
#endif
}
