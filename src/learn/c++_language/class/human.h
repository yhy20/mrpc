#ifndef __MY_HUMAN__
#define __MY_HUMAN__

#include <string>
#include <iostream>

namespace nmsp {
	class Human {
	public:
		Human();
		Human(int, const std::string&);
		void human_func();
		virtual void eat();
		
	public:
		int m_age;
		std::string m_name;
	};
}

#endif
