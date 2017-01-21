#include <iostream>

#include "bind.h"

using namespace std;

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

struct noncopyable
{
    noncopyable() = default;

    noncopyable(noncopyable const&) = delete;
    noncopyable(noncopyable&&) = default;

    noncopyable& operator=(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable&&) = default;
};

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

int test_7(my_obj& e, int r) {
	return e.val -= r - 10000;
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
    
    //cout << call(test_1, cl_4) << "\n";
    
    cout << "f1:\n";
    
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
    
    cout << bind(f, 42, _1, bind(sum, _1, 100000000))(123, 345) << "   !!!\n";
    
    
    bind([](noncopyable const&, int) {}, noncopyable{}, _1)(5);
    
    cout << "-----------------------------\n";
    cout << "f6:\n";
    
    my_obj c10(30);
    
    auto f6 = bind(test_7, _1, 6);
    cout << "$$$$\n";
    cout << f6(c10, 4) << "\n";
    
    cout << c10.val << "\n";
    
    return 0;
}
