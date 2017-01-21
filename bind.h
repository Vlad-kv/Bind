
#ifndef BIND_H
#define BIND_H

#include <iostream>

using namespace std;


template <size_t n>
struct place_holder{
};

place_holder<1> _1;
place_holder<2> _2;
place_holder<3> _3;
place_holder<4> _4;
place_holder<5> _5;
place_holder<6> _6;
place_holder<7> _7;
place_holder<8> _8;
place_holder<9> _9;

///-----------------------------------------------------------

template<typename... Args>
struct compile_list;

template<>
struct compile_list<> {
	const static int size = 0;
};

template <typename First, typename... Other>
struct compile_list<First, Other...> : public compile_list<Other...> {
	
	typedef typename std::remove_reference<First>::type value_type;
	typedef compile_list<Other...> base_type;
	
	const static int size = (sizeof... (Other)) + 1;
	
	value_type value;
	base_type& base = static_cast<base_type&>(*this);
	
	compile_list(First&& first, Other&&... other)
	: compile_list<Other...>(forward<Other>(other)...), value(forward<First>(first)) {
	}
	
};

///-----------------------------------------------------


template<int number, typename First, typename... Other>
struct c_l_getter {
	
	typedef typename c_l_getter<number - 1, Other...>::return_type return_type;
	
	static return_type& get(compile_list<First, Other...>& cl) {
		return c_l_getter<number - 1, Other...>::get(cl);
	}
};

template<typename First, typename... Other>
struct c_l_getter<0, First, Other...> {
	
	typedef typename compile_list<First, Other...>::value_type return_type;
	
	static return_type& get(compile_list<First, Other...>& cl) {
		return cl.value;
	}
};

template<int number, typename First, typename... Other>
typename c_l_getter<number, First, Other...>:: return_type&
get(compile_list<First, Other...>&& cl) {
	return c_l_getter<number, First, Other...>::get(cl);
}

template<int number, typename First, typename... Other>
typename c_l_getter<number, First, Other...>:: return_type&
get(compile_list<First, Other...>& cl) {
	return c_l_getter<number, First, Other...>::get(cl);
}

///----------------------------c_l_2
///-----------------------------------
///-----------------------------------



template<typename... Args>
struct compile_list_2;

template<>
struct compile_list_2<> {
	const static int size = 0;
};

template <typename First, typename... Other>
struct compile_list_2<First, Other...> : public compile_list_2<Other...> {
	
	typedef First value_type;
	typedef compile_list_2<Other...> base_type;
	
	const static int size = (sizeof... (Other)) + 1;
	
	value_type value;
	base_type& base = static_cast<base_type&>(*this);
	
	compile_list_2(First&& first, Other&&... other)
	: compile_list_2<Other...>(forward<Other>(other)...), value(forward<First>(first)) {
	}
	
};

///-----------------------------------------------------


template<int number, typename First, typename... Other>
struct c_l_getter_2 {
	
	typedef typename c_l_getter_2<number - 1, Other...>::return_type return_type;
	
	static return_type& get(compile_list_2<First, Other...>& cl) {
		return c_l_getter_2<number - 1, Other...>::get(cl);
	}
};

template<typename First, typename... Other>
struct c_l_getter_2<0, First, Other...> {
	
	typedef typename compile_list_2<First, Other...>::value_type return_type;
	
	static return_type& get(compile_list_2<First, Other...>& cl) {
		return cl.value;
	}
};

template<int number, typename First, typename... Other>
typename c_l_getter_2<number, First, Other...>:: return_type&
get(compile_list_2<First, Other...>&& cl) {
	return c_l_getter_2<number, First, Other...>::get(cl);
}

template<int number, typename First, typename... Other>
typename c_l_getter_2<number, First, Other...>:: return_type&
get(compile_list_2<First, Other...>& cl) {
	return c_l_getter_2<number, First, Other...>::get(cl);
}


///--------------------------------------------------------------

template <typename F, typename Compile_List, bool is_finished, int num_args, int... N>
struct call_impl {
	
	auto static call(F f, Compile_List&& c_l) {
		return call_impl<F, Compile_List, num_args == sizeof...(N) + 1,
		        num_args, N..., sizeof...(N)
		        >::call(f, std::forward<Compile_List>(c_l));
	}
};

template <typename F, typename Compile_List, int num_args, int... N>
struct call_impl<F, Compile_List, true, num_args, N...> {
	
	auto static call(F f, Compile_List&& c_l) {
		return f(get<N>(std::forward<Compile_List>(c_l))...);
	}
};

template <typename F, typename... Compile_List_Args>
auto call(F f, compile_list_2<Compile_List_Args...>&& c_l) {
	return call_impl<F, compile_list_2<Compile_List_Args...>, 0 == sizeof...(Compile_List_Args), sizeof...(Compile_List_Args)
						>::call(f, std::forward<compile_list_2<Compile_List_Args...>>(c_l));
 }

template <typename F, typename... Compile_List_Args>
auto call(F f, compile_list_2<Compile_List_Args...>& c_l) {
	return call_impl<F, compile_list_2<Compile_List_Args...>, 0 == sizeof...(Compile_List_Args), sizeof...(Compile_List_Args)
						>::call(f, std::forward<compile_list_2<Compile_List_Args...>>(c_l));
}

///

template <typename F, typename... Saved_args>
struct bind_function;


template <typename T, typename received_args>
struct calculator {
	static decltype(auto) calc(T& t, received_args& c_l) {
		
		//cout << "calc 1\n";
		return t;
	}
};

template <size_t no, typename received_args>
struct calculator<place_holder<no>&, received_args> {
	static decltype(auto) calc(place_holder<no>& p_h, received_args& c_l) {
		
		//cout << "calc 2\n";
		return get<no - 1>(c_l);
	}
};

template <typename F, typename... M, typename received_args>
struct calculator<bind_function<F, M...>&, received_args> {
	static decltype(auto) calc(bind_function<F, M...>& b_f, received_args& c_l) {
		
		//cout << "calc 3\n";
		return call(b_f, c_l);
	}
};


///---------------------------------------------------------------

template <typename F, int Number, bool is_ready_to_call, typename... Args>
struct bind_function_executer;

template <typename F, int Number, typename saved_args, typename received_args, typename... Args>
struct bind_function_executer <F, Number, true, saved_args, received_args, Args...> {
	
	static auto execute(F& f, saved_args& cl_1, received_args&& cl_2, Args&&... args) {
		
		return f(args...);
	}
	
};

template <typename T> 
T make_copy(T& t){
	return t;
}

template <typename F, int Number, typename saved_args, typename received_args, typename... Args>
struct bind_function_executer <F, Number, false, saved_args, received_args, Args...> {
	
	
	static auto execute(F& f, saved_args& cl_1, received_args&& cl_2, Args&&... args) {
		typedef decltype(calculator<decltype(get<Number>(cl_1)), received_args>::calc(get<Number>(cl_1), cl_2 )) next_type;
		
		typedef typename std::remove_reference<next_type>::type real_next_type;
		
		return bind_function_executer<F, Number + 1, (Number + 1) == saved_args::size,
											saved_args, received_args, Args...,
											next_type
										>::execute(f, cl_1, forward<received_args>(cl_2), args..., calculator<decltype(get<Number>(cl_1)), received_args>::calc(get<Number>(cl_1), cl_2 ));
	}
};


///---------------------------------------------------------------

template <typename F, typename... Saved_args>
struct bind_function {
	
	compile_list<Saved_args...> saved_args;
	F m_function;
	
	bind_function(F&& f, Saved_args&&... s_args)
	: m_function(std::forward<F>(f)), saved_args(std::forward<Saved_args>(s_args)...) {
	}
	
	template <typename... Args>
	auto operator()(Args&&... args) {
		
		//compile_list<Args...> c_l(std::forward<Args>(args)...);
		//cout << "!!!!!!!!!!!!!!\n";
		return bind_function_executer<F, 0,
										0 == sizeof...(Saved_args),
										compile_list<Saved_args...>,
										compile_list_2<Args...>
										>::execute(m_function , saved_args, compile_list_2<Args...>(std::forward<Args>(args)...));
	}
	
};

template <typename F, typename... Args>
auto bind(F f, Args&&... args) {
	return bind_function<F, Args...>(std::forward<F>(f), std::forward<Args>(args)...);
}

#endif
