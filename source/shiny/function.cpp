#include <atma/xtm/bind.hpp>

#include <memory>
#include <functional>
#include <tuple>

namespace detail
{
	uint const fn_size = 32;
	uint const fn_bufsize = fn_size - sizeof(void*) - sizeof(void*);

	//using functor_buf_t = char[fn_bufsize];
	struct functor_buf_t
	{
		char pad[fn_bufsize];
	};
	
	template <typename, typename...>
	struct functor_vtable_t;

	template <typename T>
	struct is_inplace_allowed_t
	{
		static bool const value = sizeof(T) <= fn_bufsize;
	};

	template <typename R, typename... Params>
	using dispatch_fnptr_t = auto(*)(functor_buf_t&, Params...) -> R;



	template <typename T>
	auto functorize(T&& t) -> T
	{
		return t;
	}

	template <typename R, typename C, typename... Params>
	auto functorize(R (C::*fn)(Params...)) -> decltype(std::mem_fn(fn))
	{
		return std::mem_fn(fn);
	}

	template <typename R, typename C, typename... Params>
	auto functorize(R(C::*fn)(Params...) const) -> decltype(std::mem_fn(fn))
	{
		return std::mem_fn(fn);
	}



	template <typename FN, bool = is_inplace_allowed_t<FN>::value>
	struct dispatch_central_t;

	template <typename FN>
	struct dispatch_central_t<FN, true>
	{
		static auto store(functor_buf_t& buf, FN const& fn) -> void
		{
			new (&buf) FN(fn);
		}

		static auto move(functor_buf_t& dest, functor_buf_t&& src) -> void
		{
			auto&& rhs = reinterpret_cast<FN&&>(src);
			new (&dest) FN(std::move(rhs));
		}

		static auto destruct(functor_buf_t& fn) -> void
		{
			reinterpret_cast<FN&>(fn).~FN();
		}

		template <typename R, typename... Args>
		static auto call(functor_buf_t& buf, Args... args) -> R
		{
			auto& fn = reinterpret_cast<FN&>(buf);
			return fn(std::forward<Args>(args)...);
		}
	};

	template <typename FN>
	struct dispatch_central_t<FN, false>
	{
		static auto store(functor_buf_t& buf, FN const& fn) -> void
		{
			reinterpret_cast<FN*&>(buf) = new FN(fn);
		}

		static auto move(functor_buf_t& dest, functor_buf_t&& src) -> void
		{
			//auto&& lhs = reinterpret_cast<FN*&>(dest);
			//lhs = new FN(*reinterpret_cast<FN* const&>(src));
			//auto&& rhs = ;
			//lhs = rhs;
		}

		static auto destruct(functor_buf_t& buf) -> void
		{
			delete reinterpret_cast<FN*&>(buf);
		}

		template <typename R, typename... Args>
		static auto call(functor_buf_t& buf, Args... args) -> R
		{
			//return reinterpret_cast<FN*>(&buf)->operator ()(std::forward<Args>(args)...);
			return R();
		}
	};

	template <typename R, typename ParamsTuple, typename ResultParamTuple>
	struct functor_vtable_call_partial_t;

	template <typename R, typename... Params, typename... RParams>
	struct functor_vtable_call_partial_t<R, std::tuple<Params...>, std::tuple<RParams...>>
	{
		//using call_fntype = auto(*)(dispatch_fnptr_t<R, Params..., RParams...>, functor_buf_t&, Params...) -> std::function<R(RParams...)>;

		//call_fntype call;
		auto call(dispatch_fnptr_t<R, Params..., RParams...> dispatch, functor_buf_t& buf, Params... params) -> std::function<R(RParams...)>
		{
			return atma::xtm::curry(dispatch, buf, params...);
		}
	};


	template <size_t, size_t, typename R, typename ParamsTuple>
	struct functor_vtable_call_t;

	// full-call
	template <size_t S, typename R, typename... Params>
	struct functor_vtable_call_t<S, S, R, std::tuple<Params...>>
		: functor_vtable_call_t<S, S - 1, R, std::tuple<Params...>>
	{
		//using call_fntype     = auto(*)(dispatch_fnptr_t<R, Params...>, Params...) -> R;
		auto call(dispatch_fnptr_t<R, Params...> dispatch, functor_buf_t& buf, Params... params) -> R
		{
			return dispatch(buf, std::forward<Params>(params)...);
		}
		//call_fntype call;
	};

	// partial call
	template <size_t PS, size_t S, typename R, typename... Params>
	struct functor_vtable_call_t<PS, S, R, std::tuple<Params...>>
		: functor_vtable_call_t<PS, S - 1, R, std::tuple<Params...>>
		, functor_vtable_call_partial_t<R,
			atma::xtm::tuple_select_t<atma::xtm::idxs_list_t<S>, std::tuple<Params...>>,
			atma::xtm::tuple_select_t<atma::xtm::idxs_range_t<PS - S, PS>, std::tuple<Params...>>>
	{
	};

	// terminate
	template <size_t PS, typename R, typename... Params>
	struct functor_vtable_call_t<PS, 0, R, std::tuple<Params...>>
	{};

	template <typename R, typename... Params>
	struct functor_vtable_t
		: functor_vtable_call_t<sizeof...(Params), sizeof...(Params), R, std::tuple<Params...>>
	{
		using move_fntype     = auto(*)(detail::functor_buf_t&, detail::functor_buf_t&&) -> void;
		using destruct_fntype = auto(*)(detail::functor_buf_t&) -> void;

		functor_vtable_t(move_fntype move, destruct_fntype destruct)
			: move(move)
			, destruct(destruct)
		{}

		move_fntype     move;
		destruct_fntype destruct;
	};

	template <typename R, typename... Params>
	struct functor_wrapper_t
	{
		functor_wrapper_t()
			: vtable_()
		{}

		template <typename FN>
		functor_wrapper_t(FN&& fn)
			: vtable_(generate_vtable<FN, R, Params...>())
		{
			dispatch_central_t<FN>::store(buf_, std::forward<FN>(fn));
		}

		~functor_wrapper_t()
		{
			if (vtable_)
				vtable_->destruct(buf_);
		}

		template <typename... Args>
		auto call(dispatch_fnptr_t<R, Params...> fn, Args&&... args) -> 
		decltype(vtable_->functor_vtable_call_t<sizeof...(Params), sizeof...(Args), R, std::tuple<Params...>>::call(fn, buf_, std::forward<Args>(args)...))
		{
			return vtable_->functor_vtable_call_t<sizeof...(Params), sizeof...(Args), R, std::tuple<Params...>>::call(fn, buf_, std::forward<Args>(args)...);
		}

		auto move_into(functor_wrapper_t& rhs) -> void
		{
			vtable_->move(rhs.buf_, std::move(buf_));
			rhs.vtable_ = vtable_;

			vtable_->destruct(buf_);
		}

	public:
		functor_vtable_t<R, Params...>* vtable_;
		functor_buf_t buf_;
	};


	

	template <typename FN>
	auto destruct_functor(functor_buf_t& buf) -> void
	{
		dispatch_central_t<FN>::destruct(buf);
	}

	template <typename FN>
	auto move_functor(functor_buf_t& dest, functor_buf_t&& src) -> void
	{
		dispatch_central_t<FN>::move(dest, std::move(src));
	}

	template <typename FN, typename R, typename... Params>
	auto generate_vtable() -> functor_vtable_t<R, Params...>*
	{
		static auto _ = functor_vtable_t<R, Params...>{
			&dispatch_central_t<FN>::move,
			&destruct_functor<FN>
		};

		return &_;
	}
}






template <typename> struct fn_t;

template <typename R, typename... Params>
struct fn_t<R(Params...)>
{
	using dispatch_fntype = auto (*)(detail::functor_buf_t&, Params...) -> R;


	template <typename FN>
	fn_t(FN&& fn)
	{
		functor_init(detail::functorize(std::forward<FN>(fn)));
	}

	template <typename... Args>
	auto operator ()(Args&&... args) -> decltype(wrapper_.call(dispatch_, std::forward<Args>(args)...))
	{
		return wrapper_.call(dispatch_, std::forward<Args>(args)...);
	}

	auto swap(fn_t& rhs) -> void
	{
		auto tmp = detail::functor_wrapper_t{};
		rhs.wrapper_.move_into(tmp);
		wrapper_.move_into(rhs.wrapper_);
		tmp.move_into(wrapper_);
	}

private:
	template <typename FN>
	auto functor_init(FN&& fn) -> void
	{
		dispatch_ = &detail::dispatch_central_t<FN>::template call<R, Params...>;
		wrapper_ = detail::functor_wrapper_t<R, Params...>{std::forward<FN>(fn)};
	}

private:
	detail::dispatch_fnptr_t<R, Params...> dispatch_;
	detail::functor_wrapper_t<R, Params...> wrapper_;
};





int plus(int a, float b) { return a + (int)b; }

struct dragon_t
{
	int plus(int a, int b) { return a + b; }
	int plus3(int a) { return a + 3; }
};

int function_main()
{
	auto f = fn_t<int(int, float)>{&plus};
	auto r = f(3);
	auto r2 = r(4.f);

	auto d = dragon_t();


	//auto x = atma::xtm::curry(&dragon_t::plus, &d);
	//auto x2 = new decltype(x)(x);
	//auto xr = (*x2)(3, 4);

	std::function<int(int, int)> sf = atma::xtm::curry(&dragon_t::plus, &d);
	//auto r2 = sf(4, 5);

#if 0
	auto f2 = fn_t<int(int, int)>{atma::xtm::curry(&dragon_t::plus, &d)};
	auto r2 = f2(4, 5);

	auto f3 = fn_t<int(dragon_t&, int, int)>{std::mem_fn(&dragon_t::plus)};
	auto r3 = f3(d, 3, 3);

	auto f4 = fn_t<int(int)>{atma::xtm::curry(&dragon_t::plus3, &d)};
	auto r4 = f4(4);

	return sizeof(atma::xtm::curry(&dragon_t::plus3, &d));
#endif
	return 0;
}

