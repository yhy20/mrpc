#include "women.h"

namespace nmsp {

	Woman::Woman() {
			std::cout << "Man::Man() ִ�У�" << std::endl;
	}
	Woman::Woman(int age, const std::string& name) : Human(age, name) {
		std::cout << "<Man::Man(int) ִ�У�" << std::endl;
	}

	void Woman::woman_func() {
		std::cout << "women_func() ִ��!" << std::endl;
	}

	void Woman::eat() {
		std::cout << "Women eat!" << std::endl;
	}
}