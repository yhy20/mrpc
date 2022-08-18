#include "time.h"
#include <iostream>
#include <string>

namespace _nmsp {

	// 静态成员变量定义时，不需要使用 static 修饰，
	int Time::num = 0;

	void deal_time(Time) {
		std::cout << "deal_time() 执行!" << std::endl;
	}

	int Time::caculate_microsecond_from(int millisecond) {
		return millisecond * 1000;
	}

	// 使用构造函数初始化列表
	// (一种说法：初值列在分配内存时直接赋值，函数体是分配内存后赋值，相比而言初值列效率更高(未证实))
	// 初始化列表中变量的初始化顺序取决于类中变量定义的先后顺序，与初始化列表中变量的先后顺序无关
	Time::Time() : hour(0), minute(0), second(0), microsecond(0), valid(true) {
		std::cout << "Time::Time() 执行！" << std::endl;
	}

	// 使用构造函数初始列表
	Time::Time(int m) : hour(0), minute(0), second(0), microsecond(m), valid(true) {
		std::cout << "Time::Time(int) 执行！" << std::endl;
	}

	// 使用构造函数初始列表
	Time::Time(int h, int m, int s) : hour(h), minute(m), second(s), microsecond(0), valid(true) {
		std::cout << "Time::Time(int, int, int) 执行！" << std::endl;
	}

	Time::Time(const Time& rhs) {
		std::cout << "Time::Time(const Time&) 执行！" << std::endl;
		this->hour = rhs.hour;
		this->minute = rhs.minute;
		this->second = rhs.second;
		this->microsecond = rhs.microsecond;
		this->valid = rhs.valid;
	}

	Time& Time::operator=(const Time& rhs) {
		std::cout << "Time:::operator=(const Time&) 执行！" << std::endl;
		this->hour = rhs.hour;
		this->minute = rhs.minute;
		this->second = rhs.second;
		this->microsecond = rhs.microsecond;
		this->valid = rhs.valid;
		return *this;
	}
	
	Time& Time::operator++() {
		this->hour += 1;
		return *this;
	}

	Time&& Time::operator++(int) {
		Time ret = *this;
		++(*this);
		return std::move(ret);
	}

	void Time::init_time( /* Time* this, */ int h, int m, int s, int ms, bool v) {
		this->hour = h;
		this->minute = m;
		this->second = s;
		this->microsecond = 0;
		this->valid = v;
	}

	void Time::set_timer(int millisecond) {
		microsecond = caculate_microsecond_from(millisecond);
	}

	// 如果使用 int& Time::get_hour() const {} 这种形式
	// 则报错：无法从 const int 转换为 int &，也就无法在外部修改对象的状态
	const int& Time::get_hour() const {
		return this->hour;
	}

	// 如果使用 int*X Time::get_hour() const {} 这种形式
	// 则报错：无法从 const int* 转换为 int* ，也就无法在外部修改对象的状态
	const int* Time::get_minute() const {
		return &this->minute;
	}

	int Time::get_microsecond() const {
		return microsecond;
	}

	void Time::output_time() const {
		if (hour < 10) std::cout << "0" << hour << ":";
		else std::cout << hour << ":";
		if (minute < 10) std::cout << "0" << minute << ":";
		else std::cout << minute << ":";
		if (second < 10) std::cout << "0" << second << std::endl;
		else std::cout << second << std::endl;
	}

	void Time::output_micro() const {
		std::cout << "microsecond = " << microsecond << std::endl;
	}

	void print(const Time& t) {
		if (t.hour < 10) std::cout << "0" << t.hour << ":";
		else std::cout << t.hour << ":";
		if (t.minute < 10) std::cout << "0" << t.minute << ":";
		else std::cout << t.minute << ":";
		if (t.second < 10) std::cout << "0" << t.second << std::endl;
		else std::cout << t.second << std::endl;
	}
	Time call_time(Time t) {
		return t;
	}
}