#include <iostream>
#include <map>
#include <functional>

namespace _nmsp1 {
	class BiggerThanZero {
	public:
		int operator()(int tv) {
			return tv >= 0 ? tv : 0;
		}
		
		// 重载 operator() 运算符
		// 重载要求在参数的数量和类型上有区别即可
		// 注意，返回的类型不同并不能区分重载函数
		int operator()(int tv1, int tv2) {
			return tv1 + tv2;
		}

		int LessThanZero(int tv) {
			return tv <= 0 ? tv : 0;
		}
	};

	// operator() 必须是成员函数
	// void operator()(BiggerThanZero b){}
	
	// class LessThanZero {
	// public:
	//	 // operator() 运算符不能是静态成员函数
	//	 static int operator()(int tv) {
	//		 return tv <= 0 ? tv : 0;
	//	 }
	// };

	// echo_value 的形参数量、类型、返回值与 BiggerThanZero 重载的 operator() 完全相同
	// 也称为调用形式相同 int(int)
	int echo_value(int tv) {
		// std::cout << "tv = " << tv << ::std::endl;
		return tv;
	}

	int overload(int tv1) {
		return tv1 * 2;
	}

	int overload() {
		return 0;
	}
}

void test1() {
	// (1.1) operator() 函数调用运算符
	// 如果在类中重载了 operator() 运算符，则可以像使用函数一样使用这个类的对象
	// 注意 operator() 必须是成员函数且 operator() 不能是静态成员函数
	// 重载了 operator() 运算符的对象也叫函数对象（可调用对象）
	_nmsp1::BiggerThanZero obj;
	// 隐式调用 operator()
	std::cout << obj(5) << std::endl;
	// 显式调用 operator()
	std::cout << obj.operator()(5) << std::endl;
	// 生成临时对象隐式调用调用 operator()
	std::cout << _nmsp1::BiggerThanZero()(5) << std::endl;
	// 生成临时对象显式调用调用 operator()
	std::cout << _nmsp1::BiggerThanZero().operator()(5)<< std::endl;
	std::cout << std::endl;
	// (1.2) 不同调用对象的调用形式相同
	// 函数 echo_value 与 BiggerThanZero 重载的 operator() 调用形式完全相同
	// 一种调用形式对应一种函数类型，函数类型由返回值和形参共同决定
	// 函数 echo_value 与 BiggerThanZero 重载的 operator() 的函数类型都是  int(int)
	
	// 可调用对象包括普通函数、函数指针、lambda 表达式、bind 对象、函数对象
	// 用统一的方式将各种相同调用形式可调用对象保存起来
	// 定义一个函数指针
	int(*p)(int) = _nmsp1::echo_value;
	// 定义一种类型
	typedef int(*func)(int);
	func p1 = _nmsp1::echo_value;

	// 假如需要用容器保存各种可调用对象
	std::map<std::string, func> mp;
	mp.insert({ "1) int(int)", p });
	mp.insert({ "2) int(int)", _nmsp1::echo_value});
	// 遇到一个问题，如何把一个函数对象保存进容器，显然下面的代码不可行
	// mp.insert({"3) int(int)", _nmsp1::BiggerThanZero})
	// 更进一步，如何把一个成员函数保存进容器

	// (1.3) function 类模板
	// 传递给 function 的模板参数表示 function 能管理的可调用对象的调用形式
	// a) 函数指针
	std::function<int(int)> f1 = p;
	// b) 普通函数
	std::function<int(int)> f2 = _nmsp1::echo_value;
	// c) 函数对象
	std::function<int(int)> f3 = _nmsp1::BiggerThanZero();
	// d) lambda
	std::function<int(int)> f4 = [](int tv)->int { return tv * 2; };
	// e) bind 对象
	std::function<int(int)> f5 = std::bind(&_nmsp1::BiggerThanZero::LessThanZero, &obj, std::placeholders::_1);

	std::map<std::string, std::function<int(int)>> mp1 = {
		{"f1", p},
		{"f2", _nmsp1::echo_value},
		{"f3", _nmsp1::BiggerThanZero()},
		{"f4", [](int tv)->int { return tv * 2; }},
		{"f5", std::bind(&_nmsp1::BiggerThanZero::LessThanZero, &obj, std::placeholders::_1)}
	};
	std::cout << "f1(10) = " << mp1["f1"](10) << std::endl;
	std::cout << "f2(10) = " << mp1["f2"](10) << std::endl;
	std::cout << "f3(10) = " << mp1["f3"](10) << std::endl;
	std::cout << "f4(10) = " << mp1["f4"](10) << std::endl;
	std::cout << "f5(10) = " << mp1["f5"](10) << std::endl;

	// function 遇到普通函数重载时将无法识别是哪一个函数
	// 也就是说模板参数不能区分函数形参（有二义性）
	// std::function<int(int)> fv = _nmsp1::overload;

	// 解决方法是使用函数指针
	int(*p2)(int) = _nmsp1::overload;
	int(*p3)() = _nmsp1::overload;
	// 通过传递函数指针间接的传递 overload() 函数的正确版本
	std::function<int(int)> fv = p2;
}

void test2() {

}

void test3() {

}

void test4() {
	// (4.1) 类型萃取概述（type traits) (C++11)
	// 类型萃取技巧在 STL 源码实现中应用较多，主要用于提取一些信息
}

int main() {
	// 一、function
	// test1();

	// 二、可调用对象与bind
	// test2();

	// 三、
	// test3();

	// 四、traits
	test4();
	return 0;
}