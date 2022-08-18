#include <iostream>

namespace {
	int g_count = 2;
}

namespace nmsp2 {
	void print_nmsp2_count() {
		std::cout << "nmsp2.cpp::g_count = " << g_count << std::endl;
	}
}
