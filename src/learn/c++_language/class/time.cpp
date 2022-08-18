#include "time.h"
#include <iostream>
#include <string>

namespace _nmsp {

	// ��̬��Ա��������ʱ������Ҫʹ�� static ���Σ�
	int Time::num = 0;

	void deal_time(Time) {
		std::cout << "deal_time() ִ��!" << std::endl;
	}

	int Time::caculate_microsecond_from(int millisecond) {
		return millisecond * 1000;
	}

	// ʹ�ù��캯����ʼ���б�
	// (һ��˵������ֵ���ڷ����ڴ�ʱֱ�Ӹ�ֵ���������Ƿ����ڴ��ֵ����ȶ��Գ�ֵ��Ч�ʸ���(δ֤ʵ))
	// ��ʼ���б��б����ĳ�ʼ��˳��ȡ�������б���������Ⱥ�˳�����ʼ���б��б������Ⱥ�˳���޹�
	Time::Time() : hour(0), minute(0), second(0), microsecond(0), valid(true) {
		std::cout << "Time::Time() ִ�У�" << std::endl;
	}

	// ʹ�ù��캯����ʼ�б�
	Time::Time(int m) : hour(0), minute(0), second(0), microsecond(m), valid(true) {
		std::cout << "Time::Time(int) ִ�У�" << std::endl;
	}

	// ʹ�ù��캯����ʼ�б�
	Time::Time(int h, int m, int s) : hour(h), minute(m), second(s), microsecond(0), valid(true) {
		std::cout << "Time::Time(int, int, int) ִ�У�" << std::endl;
	}

	Time::Time(const Time& rhs) {
		std::cout << "Time::Time(const Time&) ִ�У�" << std::endl;
		this->hour = rhs.hour;
		this->minute = rhs.minute;
		this->second = rhs.second;
		this->microsecond = rhs.microsecond;
		this->valid = rhs.valid;
	}

	Time& Time::operator=(const Time& rhs) {
		std::cout << "Time:::operator=(const Time&) ִ�У�" << std::endl;
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

	// ���ʹ�� int& Time::get_hour() const {} ������ʽ
	// �򱨴��޷��� const int ת��Ϊ int &��Ҳ���޷����ⲿ�޸Ķ����״̬
	const int& Time::get_hour() const {
		return this->hour;
	}

	// ���ʹ�� int*X Time::get_hour() const {} ������ʽ
	// �򱨴��޷��� const int* ת��Ϊ int* ��Ҳ���޷����ⲿ�޸Ķ����״̬
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