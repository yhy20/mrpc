#include <iostream>
#include "function_hiding.h"

namespace parent_nmsp {

	void nmsp_func(int tv) {
		std::cout << "parent_nmsp::nmsp_func(int) 执行！" << std::endl;
	}

	void nmsp_func(const std::string&) {
		std::cout << "parent_nmsp::nmsp_func(const std::string&) 执行！" << std::endl;
	}

	void nmsp_func(int, const std::string&) {
		std::cout << "parent_nmsp::nmsp_func(int, const std::string&) 执行！" << std::endl;
	}

	void nmsp_func(int, int, int) {
		std::cout << "parent_nmsp::nmsp_func(int, int, int) 执行！" << std::endl;
	}

	namespace sub_nmsp{
		void nmsp_func(int, int, int) {
			std::cout << "parent_nmsp::sub_nmsp::nmsp_func(int, int, int) 执行！" << std::endl;
		}

		void test() {
			// 试图直接调用父命名空间的重载函数会报错
			// nmsp_func(1);
			// 加上作用域解析符号可以成功调用	
			parent_nmsp::nmsp_func(1);

			// 试图直接调用父命名空间的重载函数会报错
			// nmsp_func("Hello1!");
			// 加上作用域解析符号可以成功调用
			parent_nmsp::nmsp_func("Hello!");

			// 使用 using 语句后
			using parent_nmsp::nmsp_func;
			nmsp_func(1, "Hello!");

			// 可以看出不同作用域的相同函数（同名，同形参，同返回值）之间毫无关系
			// 唯一的关系就是默认情况下子作用域的同名函数会隐藏父作用域全部同名函数(无论返回值，参数个数，参数类型是否相同)
			// 根本原因是编译器无法区分是调用父作用域的同名函数还是子作用域的同名函数
			// 使用作用域解析符显式指定调用函数所属的作用域则不会导致这种混乱
			// 从以上结论也也可以看出，函数重载只会发生在同一作用域下，而不会跨作用域
			// 也就是说，只会在类的成员函数之间、全局函数之间、同命名空间函数之间发生重载
			// 不会在父类成员函数函数与子类成员函数之间、类内成员函数与全局函数之间
			// 父命名空间函数与子命名空间函数之间发生函数重载
			// 需要特别注意
			// 1) 函数返回值类型与构成函数重载无任何关系
			// b) 类的静态成员函数与普通成员函数之间可以形成重载（因为在相同作用域内）
			nmsp_func(1, 1, 1);
			parent_nmsp::nmsp_func(1, 1, 1);
		}
	}

	void outside_class_func(int) {
		std::cout << "parent_nmsp::outside_class_func(int) 执行！" << std::endl;
	}

	void outside_class_func(const std::string&) {
		std::cout << "parent_nmsp::outside_class_func(const std::string&) 执行！" << std::endl;
	}

	void outside_class_func(int, const std::string&) {
		std::cout << "parent_nmsp::outside_class_func(const std::string&) 执行！" << std::endl;
	}

	void outside_class_func(int, int, int) {
		std::cout << "parent_nmsp::outside_class_func(int, int , int) 执行！" << std::endl;
	}


	void Base::outside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Base::outside_class_func(int, int, int) 执行！" << std::endl;
	}

	void Base::test() {
		// 试图直接调用命名空间内的同名函数会报错
		// outside_class_func(1);
		// 加上作用域解析符号可以成功调用	
		parent_nmsp::outside_class_func(1);

		// 试图直接调用命名空间内的同名函数会报错
		// outside_class_func("Hello!");
		// 加上作用域解析符号可以成功调用
		parent_nmsp::outside_class_func("Hello!");

		// 调用类内的 outside_class_func(int, int, int) 函数
		outside_class_func(1, 1, 1);
		// 调用类外的 outside_class_func(int, int, int) 函数
		parent_nmsp::outside_class_func(1, 1, 1);
	}

	void Base::inside_class_func(int) {
		std::cout << "parent_nmsp::Base::inside_class_func(int) 执行！" << std::endl;
	}

	void Base::inside_class_func(const std::string&) {
		std::cout << "parent_nmsp::Base::inside_class_func(const std::string&) 执行！" << std::endl;
	}

	void Base::inside_class_func(int, const std::string&) {
		std::cout << "parent_nmsp::Base::inside_class_func(int, const std::string&) 执行！" << std::endl;
	}

	void Base::inside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Base::inside_class_func(int, int, int) 执行！" << std::endl;
	}

	void Derived::inside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Derived::inside_class_func(int, int, int) 执行！" << std::endl;
	}

	void Derived::test() {
		// 试图直接调用父类的同名函数会报错
		// inside_class_func(1);
		// 加上作用域解析符号可以成功调用	
		Base::inside_class_func(1);

		// 试图直接调用父类的同名函数会报错
		// inside_class_func("Hello!");
		// 加上作用域解析符号可以成功调用
		Base::inside_class_func("Hello!");

		// 使用 using 语句后
		inside_class_func(1, "Hello!");

		// 调用子类的 inside_class_func(int, int, int) 函数
		inside_class_func(1, 1, 1);
		// 调用父类的 inside_class_func(int, int, int) 函数
		Base::outside_class_func(1, 1, 1);
	}
}