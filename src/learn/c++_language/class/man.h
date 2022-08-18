#ifndef __MY_MAN__
#define __MY_MAN__

#include "human.h"

namespace nmsp {
	class Man : public Human {
	public:
		Man();
		Man(int, const std::string&);
		void man_func();
		virtual void eat() override;
	};
}

#endif

