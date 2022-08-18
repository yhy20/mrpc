#include <iostream>

namespace _nmsp {
	class A {
	public:

	};

	class A1 {
	public:
		void func() {};
		void func1() {};
		void func2() {};
	};

	class A2 {
	public:
		void func() {};
		void func1() {};
		void func2() {};

	public:
		char m_a2 = 'c';
	};

	class A3 {
	public:
		void func() {};
		void func1() {};
		void func2() {};

	public:
		int m_a3 = 16;
	};
}

namespace _nmsp1 {
	class A {
	public:
		// 静态成员变量，不占用类对象的内存空间
		static int m_a;
		static int m_b;

	public:
		// （静态）成员函数，不占用类对象的内存空间
		void func();
		static void static_func();
	};

	int A::m_a = 100;
	int A::m_b = 64;

	void A::func(){
		std::cout << "func() 执行！" << std::endl;
	}

	void A::static_func() {
		std::cout << "static_func() 执行！" << std::endl;
	}

	class A1 {
	public:
		virtual void vfunc1();
		virtual void vfunc2();
	};

	void A1::vfunc1() {
		std::cout << "virtual func1() 执行！" << std::endl;
	}

	void A1::vfunc2() {
		std::cout << "virtual func2() 执行！" << std::endl;
	}

	class A2 {
	public:
		int m_a2 = 100;

	public:
		virtual void vfunc1(){}
		virtual void vfunc2(){}
	};

	class A3 {
	public:
		char m_c3 = 'c';
		int m_i3 = 100;
	};

	class A4 {
	public:
		A4(){}
		virtual ~A4() {}
		float get_value() const {
			return m_value;
		}

		static int get_count() {
			return s_count;
		}

		virtual void vfunc(){}
	

	protected:
		float m_value;
		static int s_count;
	};
}

namespace _nmsp2 {

}

void test1() {
	// (1.1) 类对象中成员变量和成员函数的空间占用情况
	_nmsp::A a;
	// 为什么 sizeof(empty class）= 1，而不是 0
	// 首先 a 对象是有地址的，只要有内存地址就表明至少能存储一个字节的数据，这就解释了 sizeof(a) 为什么为 1，而不为 0
	std::cout << "sizeof(a) = " << sizeof(a) << std::endl;

	_nmsp::A1 a1;
	// 类的成员函数不占用类对象的内存空间
	std::cout << "sizeof(a1) = " << sizeof(a1) << std::endl;

	_nmsp::A2 a2;
	// 此时 a2 对象占用的一个字节内存空间实际是 m_a2 成员变量的
	// 其实很容易理解，当对象为空时，为了在内存中表示一个对象，必须要要分配一个字节的空间占位
	// 这是因为使用对象调用成员函数时，需要向成员函数中传递对象的 this 指针，如果不分配空间的话
	// 就找不到内存中对象的地址，也就无法给成员函数传递 this 指针了，这显然是有问题的
	// 当类中有成员变量时，类对象已经有为成员变量分配的空间了，自然也不需要一个字节的空间占位
	std::cout << "sizeof(a2) = " << sizeof(a2) << std::endl;
	// 可以看到 &a2 地址处一字节内存的值为 0x63 即 6 * 16 + 3 = 99，也就是 'c' 的 ASCII 码值
	std::cout << "address of &a2 = " << &a2 << std::endl;
	std::cout <<"ASCII code of 'c' = " << int('c') << std::endl;

	_nmsp::A3 a3;
	std::cout << "sizeof(a3) = " << sizeof(a3) << std::endl;
	// 赋值后，m_a3变量的内存空间对应的值为 10 00 00 00 
	a3.m_a3 = 32;
	std::cout << "address of &a3 = " << &a3 << std::endl;

	// 结论：
	// a) 即使没有成员变量，
	// b) 类的成员函数不占用类对象的内存空间
	// c) 类的成员变量会占用类对象的内存空间
	// 成员函数占用的内存空间是属于整个类的，而成员变量占用的内存空间是属于每个对象的
}

void test2() {
	_nmsp1::A a;
	// 对象所占用的内存空间是 1 字节，说明静态成员变量不占用类对象的内存空间
	// 其实可以把静态成员变量看作具有类作用域的静态生命周期的变量，它的空间分配发生在程序开始运行时
	// 与函数内部的 static 变量一样，它随着程序开始运行而被创建，随着整个程序结束运行而被销毁，具有静态的生命周期
	std::cout << "sizeof(a) = " << sizeof(a) << std::endl;

	// 当类无成员变量时，无论有多少个虚函数，sizeof(obj) 都只占 4 个字节的内存空间
	// 当类有成员变量时，无论有多少个虚函数，sizeof(obj) 都会多占 4 个字节的内存空间
	// 原因分析：
	// 首先要明确，每定义一个虚函数，类中就会产生一个指向该虚函数的指针（如果有 n 个虚函数，则会有 n 个指针）
	// 这些指针被集中放置在类内存空间的虚函数表（virtual table【vtbl】)中，虚函数表一般被保存在可执行文件中，并在程序执
	// 行时被装载到内存中。当类中有虚函数时，编译器会在类对象中添加一个指向虚函数表的 vptr 指针，这个指针会多占 4 个字节的内存空间

	// 思考：
	// 1) 当定义普通成员函数时，不会在类中产生指向该函数的指针，这是因为在编译期间，普通成员函数只能进行静态类型绑定
	// 静态类型绑定要求在调用普通成员函数时，必须要在编译期间就能确定调用对象的类型，并且调用该类型中的成员函数
	// 当使用父类指针指向子类对象或使用父类引用绑定子类对象时，能够唯一明确的类型就是父类指针或父类引用的类型，所以
	// 在使用父类指针或父类引用调用普通成员函数时，默认的调用对象类型为父类类型，能调用的也就只有父类中的成员函数
	// 这也是为什么如果不使用虚函数，即使父类指针指向子类对象或父类引用绑定子类对象，也无法调用子类中成员函数的原因
	// 
	// 2) 当定义了虚函数时，如果直接使用类的对象调用虚函数，或者用父类指针通过作用域解析符（::）显式指定来调用虚函数，则
	// 编译器会进行静态类型绑定（因为调用对象的类型是明确的），这样的调用被称为 "实调用"，"实调用" 与普通成员函数的调用相比
	// 并无太大区别。但如果通过父类指针直接调用虚函数，则相当于告诉编译器本次函数调用的调用对象在编译期间无法确定，需要在
	// 运行时才能确定，这样的虚函数调用过程进行的动态类型绑定。当程序运行时确定调用对象的类型后，通过 vptr 指针在内存中找到
	// 具体某个类的虚函数表，再根据调用虚函数的函数名找到指向该虚函数代码段的函数指针，最终调用虚函数。通过以上过程不难想到
	// 虽然虚函数实现了多态性，让我们可以做到 "一个接口，多种实现"，但动态类型绑定必然会带来额外的性能开销，这正是所谓的鱼和熊掌不可兼得

	_nmsp1::A1 a1;
	std::cout << "sizeof(a1) = " << sizeof(a1) << std::endl;
	// 需要 注意，虚函数必须要实现才能定义对象，如果只声明却不实现虚函数，则在定义类对象时会报错
	_nmsp1::A2 a2;		// 如果在类中只声明了虚函数而不实现，并且不定义类对象，则不会报错
	std::cout << "sizeof(a2) = " << sizeof(a2) << std::endl;
	
	// 当前的 C++ 对象模型
	// 1) 非静态的成员变量属于类的对象，空间分配在类对象的内部，也就是每个类对象都有属于自己的成员变量
	// 2) 静态成员变量属于整个类，空间分配在类对象的外部，所占用的内存空间与类对象无关
	// 3) 不论是静态成员函数还是普通成员函数，空间都分配在类对象的外部，所占用的内存空间与类对象无关
	// 4) 当类中有虚函数时，编译器会在类对象中添加一个指向虚函数表的 vptr 指针，导致对象的内存空间多占用 4 个字节
	 
	// C++ 对象占用空间模型
	// (1) 非静态成员变量所占的内存总量以及这些成员变量之间进行内存对齐的额外内存开销
	// (2) 若类中有虚函数，则虚函数指针会占固定大小的内存开销
	// (3) 多重继承下的内存模型的特殊情况

	// 总结：
	// 1) 
	// 2)
	// 3)
	// 4) 
	// 5) 如果类中有多种类型的数据成员，为了提高数据访问速度，编译器可能会进行内存对齐。这将导致类对象占据
	// 的内存空间相对大一些（比如连续的 char int 成员变量理论上占 5 字节内存空间，实际上占 8 字节内存空间 
	// 6) 无论是什么类型的指针，该指针占用的内存空间大小是固定的（可能是 4 字节，也可能是 8 字节）
	// 遗留问题：内存对齐详解

	_nmsp1::A3 a3;
	std::cout << "sizeof(a3) = " << sizeof(a3) << std::endl;
	std::cout << "sizeof(char*) = " << sizeof(char*) << std::endl;
	std::cout << "sizeof(int*) = " << sizeof(int*) << std::endl;

	_nmsp1::A4 a4;
	// 理论上是 8 字节，实际上也应该是 8 字节
	std::cout << "sizeof(a4) = " << sizeof(a4) << std::endl;
}

void test3() {

}

int main() {
	// 一、类对象所占用的空间
	// test1();

	// 二、对象结构的发展和演化
	// test2();

	// this 指针调整,
	test3();
	return 0;
}