#include "human.h"

// ���ڽ���ȫ�ֱ����Ķ����Բ���
// #include "time.h"

namespace nmsp {
	Human::Human() : m_age(0), m_name("") {
		std::cout << "Human::Human() ִ�У�" << std::endl;
	}

	Human::Human(int age, const std::string& name) : m_age(age), m_name(name) {
		std::cout << "Human::Human(int) ִ�У�" << std::endl;
	}

	void Human::human_func() {
		std::cout << "human_func() ִ��!" << std::endl;
	}

	void Human::eat() {
		std::cout << "Human eat!" << std::endl;
	}
}
