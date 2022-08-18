#ifndef __MY_FUNCTION_HIDING__
#define __MY_FUNCTION_HIDING__

#include <string>

namespace parent_nmsp {
	extern void nmsp_func(int tv);
	extern void nmsp_func(const std::string&);
	extern void nmsp_func(int, const std::string&);
	extern void nmsp_func(int, int, int);
	namespace sub_nmsp {
		extern void nmsp_func(int, int, int);
		extern void test();
	}

	extern void outside_class_func(int);
	extern void outside_class_func(const std::string&);
	extern void outside_class_func(int, const std::string&);
	extern void outside_class_func(int, int, int);

	class Base {
	public:
		void outside_class_func(int, int, int);
		void test();

	public:
		// 无法使用 using 语句让类外中的同名函数在类内中可见
		// using parent_nmsp::outside_class_func;
		void inside_class_func(int);
		void inside_class_func(const std::string&);
		void inside_class_func(int, const std::string&);
		// 报错：不能重载具有相同参数类型的静态和非静态成员函数
		// void inside_class_func(int, int, int);
	public:
		// 类的静态成员函数与普通成员函数之间的重载
		static void inside_class_func(int, int, int);
	};

	class Derived : public Base {
	public:
		using Base::inside_class_func;
		void inside_class_func(int, int, int);
		void test();
	}; 
}
#endif
