#ifndef __MY_TIME__
#define __MY_TIME__

// 关于在 .h 文件中定义全局变量的问题
// 1) 结论：语法上是允许的行，但会可能导致重复定义的问题
// a) 若头文件仅被一个源文件使用到，可以正常生成可执行文件
// b) 若头文件被多个源文件包含，可正常执行完 cpp、cc1、as，但在链接(ld)时便会报错（重复定义问题）

// 2) 原因分析： 
// 首先要明确 .h 文件是提供给预处理器的，在预处理器处理完成后就不再使用了
// 所以 .h 文件的本质作用是在预处理阶段被嵌入到每个 .cpp 中提供变量声明、函数声明或类声明
// 在这个声明中包含了变量、函数和类的全部外部信息，这些信息可以帮助编译器检查语法上的错误
// 由于 .h 文件可能被多个 .cpp 文件 #include，这就导致在每个 .cpp 文件中都会创建这个全局变量
// 在编译的预处理、编译和汇编阶段都将 .cpp 文件当作一个单独的文件处理，所以并不会出现问题
// 但在链接器链接（ld）阶段，会将所有的目标文件链接整合起来，最终会发现全局变量被定义多次
// 导致二义性错误，因此与定义相关的代码的都不应该放在 .h 文件中，.h文件中只能放声明
//
// 3) 正确的做法
// a) 对于全局变量，正确的做法是在 .h 文件对应的 .cpp 文件中放变量的定义，而在 .h 文件使用 extern 
// 语句将其声明外部变量，extern 语句表示这个变量已经在某个 .cpp 文件中定义了，在当前 .cpp 文件中只需要使用即可
// b) 对于函数也是同理，函数的定义放在 .h 文件对应的 .cpp 文件中，在 .h 文件中使用 extern 语句只放声明
// c) 对于类类型比较特殊，普通成员函数或静态成员函数可以在 .h 文件中类的内部实现，或者在对应的 .cpp 文件中实现
// 在类的内部实现会提示编译器将其作为内联(inlines)函数。如果尝试在 .h 文件中实现函数，会存在二义性问题
// 对于类的静态成员变量，由于需要独立的分配空间，所以只能定义在类外，因此只能定义在 .h 文件对应的 .cpp 文件中

// 关于在 .h 文件中定义静态全局变量的问题
// 1) 结论：若在 .h 文件中定义 static 全局变量，不会出现错误，但静态变量的作用域仅在 "当前源文件"，也就是说
// 不同 .cpp 文件中定义的同名 static 全局变量，实际是独属于每个源文件的不同变量，这样的行为实际很奇怪
// 2） 首先需要知道 static 修饰的全局函数是全局静态函数，它被限定在本源码文件中，不能被本源码文件以外的代码文件调用
// 也就是说它也是独属于每个源文件，不同源文件中的 static 函数实际是完全不同的函数，故不会导致错误
         
// 在 .h 文件中定义一个全局变量
// 如果有多个 .cpp 文件 #include .h 文件，会导致二义性错误（已经证实）
// int g_count = 0;

// 在 .h 文件中定义一个 static 全局变量
// 不会出现错误（已经证实）
// static int s_count = 0;

// 在 .h 文件中定义一个全局函数
// 如果有多个 .cpp 文件 #include .  h 文件，会导致二义性错误（已经证实）
// int global_function(int tv) {
//	 return tv * tv;
// } 

// 在 .h 文件中定义一个静态全局函数
// 不会出现错误（已经证实）
// static int static_function(int tv) {
//	 return tv * tv;
// }

namespace _nmsp {
	// 即使在某个命名空间下，在 .h 文件中定义全局变量，依然会导致二义性问题
	// int g_count = 0;
	
	// 即使在某个命名空间下，在 .h 文件中定义全局函数，依然会导致二义性问题
	// int global_function(int tv) {
	//	   return tv * tv;
	// }

	// 在默认命名空间和其子命名空间下定义相同的函数不会导致重复定义问题
	// static int static_function(int tv) {
	// 	 return tv * tv;
	// }

	// 前向引用，不完整类类型声明
	class Time;
	void deal_time(Time);

	// 类定义的 .h 文件被多次 #include 时可以理解为类声明
	// 类定义，完整的类类型声明
	class Time {
	private:
		int microsecond;

	private:
		int caculate_microsecond_from(int);

	public:
		int hour;
		int minute;
		int second;
		mutable bool valid;
		
		// 声明一个静态成员变量（还未分配内存，也没有初始化）
		// 可以理解为一个拥有类作用域和静态生命周期的变量
		static int num;

	public:
		Time();
		explicit Time(int);
		explicit Time(int, int, int s = 0);
		Time(const Time&);
		Time& operator=(const Time&);
		Time& operator++();
		Time&& operator++(int);
		void init_time(int h = 0, int m = 0, int s = 0, int ms = 0, bool v = false);
		void set_timer(int);
		const int& get_hour() const;
		const int* get_minute() const;
		int get_microsecond() const;
		void output_time() const;
		void output_micro() const;

	public:
		void add_hour(int h) {
			this->hour += h;
		}
		Time* get_self() {
			return this;
		}
		/* Time* get_self(); */

	public:
		// 静态成员函数上不允许使用类型限定符
		static int get_num() /* const */{
			return num;
		}
		/* static int get_num(); */
	};

	// 尝试在 .h 文件中实现静态成员函数，存在二义性错误
	// int Time::get_num() {
	//	 return num;
	// }
	// 
	// 尝试在 .h 文件中实现普通成员函数，存在二义性错误
	// Time* Time::get_self() {
	//	 return this;
	// }

	// 尝试在 .h 文件中定义静态成员变量，存在二义性错误
	// int Time::num = 0;

	template <typename T>
	const T& operator<<(T& out, const Time& time) {
		time.output_time();
		return out;
	}
	
	// 默认是 exyern 的
	void print(const Time&);
	extern Time call_time(Time);
}

#endif
