#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/math/intersection.hpp>
#include <atma/bind.hpp>


#include <filesystem>




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
}

namespace math = atma::math;

#include <array>

template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_t
{
	octree_t() {}

	//auto add_triangle(T const&, math::vector4f const&, math::vector4f const&, math::vector4f const&) -> void;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;
	//node_t root_;

private:

private:

private:
	
};


#if 0
struct box_t
{
	math::vector4f origin;
	math::vector4f half_extents;

	static auto from_minmax(math::vector4f const& min, math::vector4f const& max) -> box_t
	{
		return box_t{(min + max) / 2.f, (max - min) / 2.f};
	}
};

auto intersect_aabb_box(math::vector4f const& aabb, box_t const& box) -> bool
{
	return !(
		aabb.x + aabb.w < box.origin.x - box.half_extents.x ||
		aabb.x - aabb.w > box.origin.x + box.half_extents.x ||
		aabb.y + aabb.w < box.origin.y - box.half_extents.y ||
		aabb.y - aabb.w > box.origin.y + box.half_extents.y ||
		aabb.z + aabb.w < box.origin.z - box.half_extents.z ||
		aabb.z - aabb.w > box.origin.z + box.half_extents.z)
		;
}
#endif





template <typename... Args>
using variadic_size = std::tuple_size<std::tuple<Args...>>;









#if 0

struct octree_t::node_t
{
	node_t()
		: bounds(0.f, 0.f, 0.f, 0.5f)
		, children_()
		, datas_()
	{
	}

	node_t(math::vector4f const& bounds)
		: bounds(bounds)
		, children_()
		, datas_()
	{
	}

	atma::math::vector4f bounds;
	triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::vector4f const& point, T const& data) -> bool;

private:
	auto imem_allocate() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];
	}

	auto imem_deallocate() -> void;


	auto oct_split() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];

		for (auto i = 0u; i != 8u; ++i)
			new (children_ + i) node_t( oct_subbound(bounds, i) );

		for (auto i = 0u; i != 8u; ++i)
			insert(dloc_[i], data_[i]);
	}

	auto oct_subbound(math::vector4f const&, uint) -> math::vector4f;

private:
	node_t* children_;

	std::array<T, 8> data_;
	std::array<math::vector4f, 8> dloc_;

	uint datas_;
};


auto octree_t::node_t::insert(triangle_t const& tri) -> bool
{
	
	if (!inbounds(point))
	{
		return false;
	}

	// we are a full leaf node
	if (!children_ && datas_ == 8)
	{
		oct_split();
	}

	if (children_)
	{
		return std::any_of(children_, children_ + 8, [&point, &data](node_t& node) {
			return node.insert(point, data);
		});
	}

	data_[datas_] = data;
	dloc_[datas_] = point;
	++datas_;

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return
		point.x < bounds.x + bounds.w && point.x > bounds.x - bounds.w &&
		point.y < bounds.y + bounds.w && point.y > bounds.y - bounds.w &&
		point.z < bounds.z + bounds.w && point.z > bounds.z - bounds.w
		;
}

auto octree_t::node_t::oct_subbound(math::vector4f const& bounds, uint idx) -> math::vector4f
{
	return math::vector4f(
		(0.5f - ((idx & 1)     ) * 1.f) * bounds.w + bounds.x,
		(0.5f - ((idx & 2) >> 1) * 1.f) * bounds.w + bounds.y,
		(0.5f - ((idx & 4) >> 2) * 1.f) * bounds.w + bounds.z,
		bounds.w * 0.5f);
}

#endif
template<typename testType>
struct is_function_pointer
{
	static const bool value =
		std::is_pointer<testType>::value ?
		std::is_function<typename std::remove_pointer<testType>::type>::value :
		false;
};

int plus2(int x) { return x + 2; }

auto const plus2fn = std::make_tuple(&plus2);

template <typename F>
struct function_t
{
	function_t(F fn) : fn_(fn) {}

	template <typename... Curried>
	auto operator ()(Curried... curried)
	-> typename std::enable_if<variadic_size<Curried...>::value == atma::function_traits<F>::arity, int>::type
	{
		return fn_(std::forward<Curried>(curried)...);
	}

	template <typename... Curried>
	auto operator ()(Curried... curried)
	-> typename std::enable_if<std::tuple_size<std::tuple<Curried...>>::value != atma::function_traits<F>::arity,
		function_t<decltype(atma::curry(fn_, std::forward<Curried>(curried)...))>>::type
	{
		return {atma::curry(fn_, std::forward<Curried>(curried)...)};
	}

	auto fn() const -> F { return fn_; }

private:
	F fn_;
};

template <typename R, typename... Args>
auto functionize(R(&fn)(Args...)) -> function_t<R(*)(Args...)>
{
	return {&fn};
}

template <typename F, typename G, typename X>
inline auto point(F f, G g, X x) -> typename atma::function_traits<F>::result_type
{
	return f(g(std::forward<X>(x)));
}

template <typename F, typename G>
struct curried_function_traits
{
	using result_type = typename atma::function_traits<F>::result_type;

	//using signature = typename atma::function_traits<F>::result_type(*)(typename atma::function_traits<G>::template arg<0>::type);

	using sg = typename atma::function_traits<F>::result_type(*)(function_t<F>, function_t<G>, typename atma::function_traits<G>::template arg<0>::type);
};



namespace atma {

	template <typename F>
	struct function_traits<function_t<F>>
		: function_traits<F>
	{};
	
#if 0
	template <typename F, typename Bindings>
	struct function_traits<bind_t<F, Bindings>>
	{
		using result_type = typename function_traits<F>::result_type;
		
		enum { arity = function_traits<F>::arity - tuple_nonplaceholder_size_t<Bindings>::value };

		template <size_t i>
		struct arg {
			typedef typename function_traits<F>::template arg<i + tuple_nonplaceholder_size_t<Bindings>::value>::type type;
		};
	};
#endif
}


#if 1
template <typename F, typename G>
inline auto operator * (function_t<F> f, function_t<G> g)
-> function_t<
	atma::bind_t<
		typename curried_function_traits<F, G>::sg,
		std::tuple<function_t<F>, function_t<G>, atma::placeholder_t<0>>
	>>
{
	static_assert(atma::function_traits<F>::arity == 1, "can not compose functions of arity greater than 1");
	static_assert(atma::function_traits<G>::arity == 1, "can not compose functions of arity greater than 1");

	auto fnptr = &point<
		function_t<F>,
		function_t<G>,
		typename atma::function_traits<G>::template arg<0>::type>;

	return {atma::curry(std::forward<decltype(fnptr)>(fnptr), std::forward<function_t<F>>(f), std::forward<function_t<G>>(g))};
}
#endif


int mul(int x, int y) { return x * y; }
int add(int x, int y) { return x + y; }

int mul2(int x) { return x * 2; }
int add4(int x) { return x + 4; }

struct giraffe
{
	int add(int x, int y) { return x + y; }
};

template <typename, typename, typename> struct Y;

template <size_t... A, size_t... B, typename Tuple>
struct Y<atma::idxs_t<A...>, atma::idxs_t<B...>, Tuple>
{ using type = std::tuple<typename std::tuple_element<A, typename std::tuple_element<B, Tuple>::type>::type...>; };





int four(int) { return 4; }

struct bb_y
{
	template <typename T>
	auto firde(T t) -> void
	{
		dynamic_cast<base_thing_t<T>*>(this)->fire_impl(t);
	}

	virtual ~bb_y() {}
};

template <typename T>
struct base_thing_t : virtual bb_y
{
	auto fire_impl(T t) -> void {}
};

enum class things_t { lol, hooray };
enum class animals_t { giraffe, dragon };


struct sizing_events_t
{
	int resize;
	int resize_dc;
	int move;
};

enum class sizing_event_type_t
{
	resize,
	resize_dc,
	move,
};


#include <atma/event.hpp>
#include <functional>
#include <list>
#include <set>
#include <mutex>


template <typename T>
struct null_combiner_t
{
	null_combiner_t()
		: result_()
	{}

	using result_type = T;

	auto reset() -> void { result_ = T(); }
	auto push(T const& x) -> void { result_ += x; }
	auto result() -> T { return result_; }

private:
	T result_;
};

template <>
struct null_combiner_t<void>
{
	null_combiner_t()
	{}

	using result_type = void;

	auto reset() -> void {}
	auto push(void) -> void { }
	auto result() const -> void { return; }
};

template <typename T>
struct count_combiner_t
{
	count_combiner_t()
		: calls_()
	{}

	using result_type = uint;

	auto reset() -> void { calls_ = 0u; }
	template <typename Y> auto push(Y const&) -> void { ++calls_; }
	auto push(void) -> void { ++calls_; }
	auto result() -> uint { return calls_; }

private:
	uint calls_;
};

namespace detail
{
	template <typename FN>
	using delegate_t = std::function<FN>;

	template <typename FN>
	using delegates_t = std::list<delegate_t<FN>>;



	template <typename R, template <typename> class CMB, typename FN, typename... Args>
	struct fire_impl_t
	{
		static auto go(CMB<R>& cmb, delegates_t<FN> const& delegates, Args&&... args) -> typename CMB<R>::result_type
		{
			cmb.reset();

			for (auto const& x : delegates)
				cmb.push(x(std::forward<Args>(args)...));

			return cmb.result();
		}
	};
	

	template <template <typename> class CMB, typename FN, typename... Args>
	struct fire_impl_t<void, CMB, FN, Args...>
	{
		static auto go(CMB<void>& cmb, delegates_t<FN> const& delegates, Args&&... args) -> typename CMB<void>::result_type
		{
			cmb.reset();

			for (auto const& x : delegates) {
				x(std::forward<Args>(args)...);
				cmb.push();
			}

			return cmb.result();
		}
	};

}

template <bool Use> struct maybe_mutex_t;
template <bool Use> struct maybe_scoped_lock_t;

template <>
struct maybe_mutex_t<true>
{
	maybe_mutex_t() {}
	maybe_mutex_t(maybe_mutex_t const&) = delete;

private:
	std::mutex mutex;

	friend struct maybe_scoped_lock_t<true>;
};

template <>
struct maybe_mutex_t<false>
{
	maybe_mutex_t(maybe_mutex_t const&) = delete;
};

template <>
struct maybe_scoped_lock_t<true>
{
	maybe_scoped_lock_t(maybe_mutex_t<true>& mtx)
		: guard_(mtx.mutex)
	{}

private:
	std::lock_guard<std::mutex> guard_;
};

template <>
struct maybe_scoped_lock_t<false>
{
	maybe_scoped_lock_t(maybe_mutex_t<false> const& mtx)
	{}
};


template <typename FN, template <typename> class CMB = count_combiner_t, bool ThreadSafe = true>
struct event_t
{
	using fnptr_type = std::decay_t<FN>;
	using fn_result_type = typename atma::function_traits<fnptr_type>::result_type;

	using delegate_t = std::function<FN>;
	using delegates_t = detail::delegates_t<FN>;
	using delegate_handle_t = typename delegates_t::const_iterator;

	using combiner_t = CMB<fn_result_type>;
	using result_type = typename combiner_t::result_type;

	
	event_t()
	{}

	event_t(combiner_t const& cmb)
		: combiner_{cmb}
	{}

	event_t(event_t const& rhs)
		: combiner_{rhs.combiner_}, mutex_{}, delegates_(rhs.delegates_)
	{}

	template <typename... Args>
	auto fire(Args&&... args) -> result_type
	{
		auto SL = maybe_scoped_lock_t<ThreadSafe>{mutex_};

		return detail::fire_impl_t<fn_result_type, CMB, FN, Args...>::go(combiner_, delegates_, std::forward<Args>(args)...);
	}

	auto operator += (delegate_t const& fn) -> delegate_handle_t
	{
		auto SL = maybe_scoped_lock_t<ThreadSafe>{mutex_};

		delegates_.push_back(fn);
		return --delegates_.end();
	}

	auto operator -= (delegate_handle_t const& fn) -> void
	{
		auto SL = maybe_scoped_lock_t<ThreadSafe>{mutex_};

		delegates_.erase(fn);
	}

private:
	

private:
	combiner_t combiner_;
	delegates_t delegates_;
	maybe_mutex_t<ThreadSafe> mutex_;
};


template <typename Range, typename C, typename M, typename T>
struct propper_t;

template <typename Range, typename C, typename M, typename... Args>
struct propper_t<Range, C, M, std::tuple<Args...>>
{
	static auto go(Range&& range, M C::*member, Args... args) -> typename M::result_type
	{
		for (auto&& x : range)
			(x.*member).fire(std::forward<Args>(args)...);

		return typename M::result_type();
	}
};

template <typename Range, typename C, typename M>
auto broadcast_across(Range&& range, M C::*member) 
-> typename M::delegate_t
{
	using params = typename atma::function_traits<typename M::fnptr_type>::tupled_args_type;

	return atma::curry(&propper_t<Range, C, M, params>::go, std::ref(range), member);
}





struct dragon_t
{
	dragon_t() {}
	
	event_t<int(int)> on_breathe_fire;

	int plus(int a, int b) { return a + b; }

	std::vector<dragon_t> children_;
};

int plus(int a, int b) { return a + b; }

template <typename C, typename FN>
auto for_each_tuple(C&& container, FN&& fn) -> void
{
	for (auto&& x : container)
	{
		atma::call_fn_tuple(std::forward<FN>(fn), std::forward<decltype(x)>(x));
	}
}

int main()
{
	auto nn = std::vector<int>{1, 2, 3, 4};
	auto nn2 = std::vector<int>{10, 20, 30, 40, 50, 60};
	auto zr = atma::zip(nn, nn2);

	auto zi = zr.begin();
	auto ze = zr.end();
	++zi;
	auto zeq = zi == ze;

	for_each_tuple(zr, [](int x, int y) {
		std::cout << (x + y) << std::endl;
	});

	for (auto const& x : zr)
	{
		std::cout << std::get<0>(x)  << " : " << std::get<1>(x) << std::endl;
	}

	event_t<int(int, int), null_combiner_t> e;

	{
		e += [](int x, int y) -> int { return x + y; };
		e += [](int x, int y) -> int { return x * y; };
		auto r = e.fire(4, 5);
	}

	event_t<void(int, int)> e2;
	int z = 0;
	e += [&](int x, int y) { return z = x + y; };



	{
		auto d = dragon_t{};
		d.children_.push_back(dragon_t());
		d.children_.back().on_breathe_fire += [](int g) -> int { std::cout << g << std::endl; return 0; };
		d.children_.back().on_breathe_fire += [](int g) -> int { std::cout << "lulz" << std::endl; return 0; };
		d.children_.back().on_breathe_fire += [](int g) -> int { std::cout << "zomg" << std::endl; return 0; };
		d.children_.back().on_breathe_fire += [](int g) -> int { std::cout << "stop setting things on fire" << std::endl; return 0; };
		d.children_.back().on_breathe_fire.fire(88);
		d.on_breathe_fire += broadcast_across(d.children_, &dragon_t::on_breathe_fire);
		d.on_breathe_fire.fire(8);
	}






	auto numbers = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	auto gt_2 =   [](int x) { return x > 2; };

	auto is_even = [](int x) { return x % 2 == 0; };
	auto is_odd = [](int x) { return x % 2 != 0; };
	auto filtered = atma::filter(is_even, numbers);
	auto numbers2 = std::vector<int>(filtered.begin(), filtered.end());
	auto plus_1 = [](int x) { return x + 1; };
	auto mapped = atma::map(plus_1, filtered);
	auto numbers3 = std::vector<int>(mapped.begin(), mapped.end());

	//auto fn_add = function_t<int (*)(int, int)>{&add};
	auto fn_add = functionize(add);
	auto fn_mul2 = functionize(mul2);
	//auto am = fn_mul2 * fn_add(4) * fn_add(1);
	//auto r = am(3);

	auto thing = atma::filter(is_even) <<= atma::map(plus_1) <<= atma::filter(is_odd) <<= numbers;
	auto thing2 = std::vector<int>(thing.begin(), thing.end());

	using T1 = std::tuple<int, char, double, float>;
	using T2 = std::tuple<long, short, uint>;

	auto tt = atma::tuple_push_back(T2(), int(4));
	auto tt2 = atma::tuple_cat(T2(), T1());

	{
		//auto mf = functionize(&plus);
		int f = 5;
		auto args = atma::bind_arguments(std::make_tuple(arg2, arg1), std::forward_as_tuple(f,4));
		//auto bngs = atma::bind(&add, f, arg1)(4, 8);
		giraffe g;
		auto cngs = atma::call_fn_bound_tuple(&giraffe::add, std::make_tuple(&g, arg2, arg1), std::make_tuple(2, 3));
	}

	using T3 = Y<atma::idxs_t<0, 1, 2>, atma::idxs_t<0, 0, 1>, std::tuple<T1, T2>>::type;
	auto t3i = T3();
}
