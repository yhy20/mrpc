#include "women.h"

namespace nmsp {

	Woman::Woman() {
			std::cout << "Man::Man() Ö´ÐÐ£¡" << std::endl;
	}
	Woman::Woman(int age, const std::string& name) : Human(age, name) {
		std::cout << "<Man::Man(int) Ö´ÐÐ£¡" << std::endl;
	}

	void Woman::woman_func() {
		std::cout << "women_func() Ö´ÐÐ!" << std::endl;
	}

	void Woman::eat() {
		std::cout << "Women eat!" << std::endl;
	}
}