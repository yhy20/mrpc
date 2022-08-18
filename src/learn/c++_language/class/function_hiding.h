#ifndef __MY_FUNCTION_HIDING__
#define __MY_FUNCTION_HIDING__

#include <string>

namespace parent_nmsp {
	extern void nmsp_func(int tv);
	extern void nmsp_func(const std::string&);
	extern void nmsp_func(int, const std::string&);
	extern void nmsp_func(int, int, int);
	namespace sub_nmsp {
		extern void nmsp_func(int, int, int);
		extern void test();
	}

	extern void outside_class_func(int);
	extern void outside_class_func(const std::string&);
	extern void outside_class_func(int, const std::string&);
	extern void outside_class_func(int, int, int);

	class Base {
	public:
		void outside_class_func(int, int, int);
		void test();

	public:
		// �޷�ʹ�� using ����������е�ͬ�������������пɼ�
		// using parent_nmsp::outside_class_func;
		void inside_class_func(int);
		void inside_class_func(const std::string&);
		void inside_class_func(int, const std::string&);
		// �����������ؾ�����ͬ�������͵ľ�̬�ͷǾ�̬��Ա����
		// void inside_class_func(int, int, int);
	public:
		// ��ľ�̬��Ա��������ͨ��Ա����֮�������
		static void inside_class_func(int, int, int);
	};

	class Derived : public Base {
	public:
		using Base::inside_class_func;
		void inside_class_func(int, int, int);
		void test();
	}; 
}
#endif
