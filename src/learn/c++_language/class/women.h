#ifndef __MY_WOMEN__
#define __MY_WOMEN__

#include "human.h"

namespace nmsp {
	class Woman : public Human {
	public:
		Woman();
		Woman(int, const std::string&);
		void woman_func();
		virtual void eat() override;
	};
}

#endif