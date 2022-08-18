#include <iostream>

namespace {
	int g_count = 1;
}

namespace nmsp1 {
	void print_nmsp1_count() {
		std::cout << "nmsp1.cpp::g_count = " << g_count << std::endl;
	}
}
