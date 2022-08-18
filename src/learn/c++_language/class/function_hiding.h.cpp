#include <iostream>
#include "function_hiding.h"

namespace parent_nmsp {

	void nmsp_func(int tv) {
		std::cout << "parent_nmsp::nmsp_func(int) ִ�У�" << std::endl;
	}

	void nmsp_func(const std::string&) {
		std::cout << "parent_nmsp::nmsp_func(const std::string&) ִ�У�" << std::endl;
	}

	void nmsp_func(int, const std::string&) {
		std::cout << "parent_nmsp::nmsp_func(int, const std::string&) ִ�У�" << std::endl;
	}

	void nmsp_func(int, int, int) {
		std::cout << "parent_nmsp::nmsp_func(int, int, int) ִ�У�" << std::endl;
	}

	namespace sub_nmsp{
		void nmsp_func(int, int, int) {
			std::cout << "parent_nmsp::sub_nmsp::nmsp_func(int, int, int) ִ�У�" << std::endl;
		}

		void test() {
			// ��ͼֱ�ӵ��ø������ռ�����غ����ᱨ��
			// nmsp_func(1);
			// ����������������ſ��Գɹ�����	
			parent_nmsp::nmsp_func(1);

			// ��ͼֱ�ӵ��ø������ռ�����غ����ᱨ��
			// nmsp_func("Hello1!");
			// ����������������ſ��Գɹ�����
			parent_nmsp::nmsp_func("Hello!");

			// ʹ�� using ����
			using parent_nmsp::nmsp_func;
			nmsp_func(1, "Hello!");

			// ���Կ�����ͬ���������ͬ������ͬ����ͬ�βΣ�ͬ����ֵ��֮����޹�ϵ
			// Ψһ�Ĺ�ϵ����Ĭ����������������ͬ�����������ظ�������ȫ��ͬ������(���۷���ֵ���������������������Ƿ���ͬ)
			// ����ԭ���Ǳ������޷������ǵ��ø��������ͬ�������������������ͬ������
			// ʹ���������������ʽָ�����ú����������������򲻻ᵼ�����ֻ���
			// �����Ͻ���ҲҲ���Կ�������������ֻ�ᷢ����ͬһ�������£��������������
			// Ҳ����˵��ֻ������ĳ�Ա����֮�䡢ȫ�ֺ���֮�䡢ͬ�����ռ亯��֮�䷢������
			// �����ڸ����Ա���������������Ա����֮�䡢���ڳ�Ա������ȫ�ֺ���֮��
			// �������ռ亯�����������ռ亯��֮�䷢����������
			// ��Ҫ�ر�ע��
			// 1) ��������ֵ�����빹�ɺ����������κι�ϵ
			// b) ��ľ�̬��Ա��������ͨ��Ա����֮������γ����أ���Ϊ����ͬ�������ڣ�
			nmsp_func(1, 1, 1);
			parent_nmsp::nmsp_func(1, 1, 1);
		}
	}

	void outside_class_func(int) {
		std::cout << "parent_nmsp::outside_class_func(int) ִ�У�" << std::endl;
	}

	void outside_class_func(const std::string&) {
		std::cout << "parent_nmsp::outside_class_func(const std::string&) ִ�У�" << std::endl;
	}

	void outside_class_func(int, const std::string&) {
		std::cout << "parent_nmsp::outside_class_func(const std::string&) ִ�У�" << std::endl;
	}

	void outside_class_func(int, int, int) {
		std::cout << "parent_nmsp::outside_class_func(int, int , int) ִ�У�" << std::endl;
	}


	void Base::outside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Base::outside_class_func(int, int, int) ִ�У�" << std::endl;
	}

	void Base::test() {
		// ��ͼֱ�ӵ��������ռ��ڵ�ͬ�������ᱨ��
		// outside_class_func(1);
		// ����������������ſ��Գɹ�����	
		parent_nmsp::outside_class_func(1);

		// ��ͼֱ�ӵ��������ռ��ڵ�ͬ�������ᱨ��
		// outside_class_func("Hello!");
		// ����������������ſ��Գɹ�����
		parent_nmsp::outside_class_func("Hello!");

		// �������ڵ� outside_class_func(int, int, int) ����
		outside_class_func(1, 1, 1);
		// ��������� outside_class_func(int, int, int) ����
		parent_nmsp::outside_class_func(1, 1, 1);
	}

	void Base::inside_class_func(int) {
		std::cout << "parent_nmsp::Base::inside_class_func(int) ִ�У�" << std::endl;
	}

	void Base::inside_class_func(const std::string&) {
		std::cout << "parent_nmsp::Base::inside_class_func(const std::string&) ִ�У�" << std::endl;
	}

	void Base::inside_class_func(int, const std::string&) {
		std::cout << "parent_nmsp::Base::inside_class_func(int, const std::string&) ִ�У�" << std::endl;
	}

	void Base::inside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Base::inside_class_func(int, int, int) ִ�У�" << std::endl;
	}

	void Derived::inside_class_func(int, int, int) {
		std::cout << "parent_nmsp::Derived::inside_class_func(int, int, int) ִ�У�" << std::endl;
	}

	void Derived::test() {
		// ��ͼֱ�ӵ��ø����ͬ�������ᱨ��
		// inside_class_func(1);
		// ����������������ſ��Գɹ�����	
		Base::inside_class_func(1);

		// ��ͼֱ�ӵ��ø����ͬ�������ᱨ��
		// inside_class_func("Hello!");
		// ����������������ſ��Գɹ�����
		Base::inside_class_func("Hello!");

		// ʹ�� using ����
		inside_class_func(1, "Hello!");

		// ��������� inside_class_func(int, int, int) ����
		inside_class_func(1, 1, 1);
		// ���ø���� inside_class_func(int, int, int) ����
		Base::outside_class_func(1, 1, 1);
	}
}