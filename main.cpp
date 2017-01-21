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
auto call(F f, compile_list<Compile_List_Args...>&& c_l) {
	return call_impl<F, compile_list<Compile_List_Args...>, 0 == sizeof...(Compile_List_Args), sizeof...(Compile_List_Args)
						>::call(f, std::forward<compile_list<Compile_List_Args...>>(c_l));
 }

template <typename F, typename... Compile_List_Args>
auto call(F f, compile_list<Compile_List_Args...>& c_l) {
	return call_impl<F, compile_list<Compile_List_Args...>, 0 == sizeof...(Compile_List_Args), sizeof...(Compile_List_Args)
						>::call(f, std::forward<compile_list<Compile_List_Args...>>(c_l));
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
	
	static auto execute(F f, saved_args& cl_1, received_args& cl_2, Args&&... args) {
		
		return f(args...);
	}
	
};

template <typename T> 
T make_copy(T& t){
	return t;
}

template <typename F, int Number, typename saved_args, typename received_args, typename... Args>
struct bind_function_executer <F, Number, false, saved_args, received_args, Args...> {
	
	
	static auto execute(F f, saved_args& cl_1, received_args& cl_2, Args&&... args) {
		typedef decltype(calculator<decltype(get<Number>(cl_1)), received_args>::calc(get<Number>(cl_1), cl_2 )) next_type;
		
		typedef typename std::remove_reference<next_type>::type real_next_type;
		
		return bind_function_executer<F, Number + 1, (Number + 1) == cl_1.size,
											saved_args, received_args, Args...,
											next_type
										>::execute(f, cl_1, cl_2, args..., calculator<decltype(get<Number>(cl_1)), received_args>::calc(get<Number>(cl_1), cl_2 ));
	}
};


///---------------------------------------------------------------

template <typename F, typename... Saved_args>
struct bind_function {
	
	compile_list<Saved_args...> saved_args;
	F&& m_function;
	
	bind_function(F&& f, Saved_args&&... s_args)
	: m_function(std::forward<F>(f)), saved_args(std::forward<Saved_args>(s_args)...) {
	}
	
	template <typename... Args>
	auto operator()(Args&&... args) {
		
		compile_list<Args...> c_l(std::forward<Args>(args)...);
		
		return bind_function_executer<F, 0,
										0 == sizeof...(Saved_args),
										compile_list<Saved_args...>,
										compile_list<Args...>
										>::execute(m_function , saved_args, c_l);
	}
	
};

template <typename F, typename... Args>
auto bind(F&& f, Args&&... args) {
	return bind_function<F, Args...>(f, std::forward<Args>(args)...);
}


int test_1(int c1, int c2, int c3, int c4) {
	return c1 * c2 * c3 + c4;
}

int test_2(int c1) {
	return c1 * 5;
}

int test_3(int c1, int c2) {
	return c1 - c2;
}

int test_4(int c1, int c2, int c3) {
	return c1 + c2 + 5 * c3 - 19;
}

struct my_obj{
	int val;
	
	my_obj(int val) : val(val) {
		cout << "+ 1\n";
	}
	
	my_obj(const my_obj& c) {
		cout << "copy_2\n";
		val = c.val;
		//return *this;
	}
	
	my_obj(const my_obj&& c) {
		cout << "copy_3\n";
		val = c.val;
		//return *this;
	}
	
	my_obj& operator=(const my_obj& c) {
		cout << "copy_1\n";
		val = c.val;
		return *this;
	}
	
	my_obj& operator=(const my_obj&& c) {
		cout << "move\n";
		
		val = c.val;
		return *this;
	}
	
	~my_obj() {
		cout << "- 1  (" << val << ")\n";
	}
};

int test_5(my_obj& c1, my_obj& c2) {
	return c1.val * c2.val;
}

int test_6(int& c) {
	return c += 4;
}

int f(int w, int e, int r) {
	return w + e + r;
}

int sum(int w, int e) {
	return w + e;
}

int main() {
    compile_list<place_holder<1>&> cl_9999(_1);
    
    
    compile_list<int, int, int, double, double, double> cl_6(1, 2, 3, 0.5, -2, 1e2);
    
    cout << cl_6.value << " " << cl_6.base.value << "\n";
    
    
    cout << get<0>(cl_6) << " " << get<5>(cl_6) << "\n";
    
    
    compile_list<int> cl_1(-9);
    compile_list<> cl_0;
    
    cout << get<0>(cl_1) << "\n";
    
//    cout << get_size(cl) << " " << get_size(cl2) << " " << get_size(cl_0) << "   !!!\n";
    
    
    compile_list<int, int, int, int> cl_4(2, 3, 4, 5);
    
    cout << call(test_1, cl_4) << "\n";
    
    cout << "f2:\n";
    
    auto f1 = bind(test_4, 3, 4, -8);
    
    cout << f1() << "\n";
    
    cout << "f2:\n";
    auto f2 = bind(test_3, _2, _1);
	cout << f2(8, 100) << "\n";
    
    cout << "f3:\n";
    auto f3 = bind(test_2, bind(test_2, _1));
    cout << f3(45, 21) << "\n";
    
    cout << "f3_1:\n";
    auto f3_1 = bind(test_5, my_obj(2), my_obj(3));
    
    cout << "\n\n";
    
    cout << f3_1() << "\n";
    
    cout << "-------------\n";
    
    cout << "f4:\n";
    auto f4 = bind(test_5, my_obj(2), _2);
    
    cout << f4(my_obj(6), my_obj(7)) << "\n";
    
    cout << "----------\n";
    
    cout << "f5:\n";
    int c = 9;
    auto f5 = bind(test_6, c);
    
    cout << f5() << "\n";
    
    cout << f5() << " " << f5() << "\n";
    
    cout << c << "\n";
    
    cout << "\n";
    
    cout << "############\n";
    
    cout << bind(f, 42, _1, bind(sum, _1, _2))(123, 345) << "   !!!\n";
    
    
    return 0;
}
