#include "human.h"

// 用于进行全局变量的二义性测试
// #include "time.h"

namespace nmsp {
	Human::Human() : m_age(0), m_name("") {
		std::cout << "Human::Human() 执行！" << std::endl;
	}

	Human::Human(int age, const std::string& name) : m_age(age), m_name(name) {
		std::cout << "Human::Human(int) 执行！" << std::endl;
	}

	void Human::human_func() {
		std::cout << "human_func() 执行!" << std::endl;
	}

	void Human::eat() {
		std::cout << "Human eat!" << std::endl;
	}
}
