#include <iostream>

namespace _nmsp1 {
	class A {
	public:

	};

	class B {
	public:
		int m_i;
	};

	class C {
	public:
		C(){}
		int m_i;
	};

	class D {
	public:
		D() :m_i(100) {}
		int m_i;
	};
}

namespace _nmsp2 {

}

void test1_1() {
	// (1.1) new �����ʱ�Ӳ������ŵĲ��
	// (1.1.1) ����������
	// a) �����һ�����࣬��������д��û�����𣬲�����ʵ�в�����ֻдһ������
	_nmsp1::A* pa1 = new _nmsp1::A;
	_nmsp1::A* pa2 = new _nmsp1::A();
	// b) ��������г�Ա��������д���캯������� () �ĳ�ʼ�����һЩ�ͳ�Ա�����йص��ڴ����㣬����������������ڴ�ȫ������
	// ĳЩ����£����������������Ĭ�Ϲ��캯����Ĭ�Ϲ��캯���Ὣ������ֵ��ʼ��Ϊ�㣬���û������Ĭ�Ϲ��캯�����������ʼֵ�������
	_nmsp1::B* pb1 = new _nmsp1::B;
	_nmsp1::B* pb2 = new _nmsp1::B();
	// c) ��������й��캯����������ĳ�ʼ�������ύ�����캯����������캯����������ʼ��������������ĳ�ʼֵ�������
	_nmsp1::C* pc1 = new _nmsp1::C;
	_nmsp1::C* pc2 = new _nmsp1::C();
	// d) ��������й��캯����������ĳ�ʼ�������ύ�����캯���������ֵ��õ�Ч����һ����
	_nmsp1::D* pd1 = new _nmsp1::D;
	_nmsp1::D* pd2 = new _nmsp1::D();

	// (1.1.2) ���ڼ�����
	int* p = new int;			// ��ֵ���
	int* p1 = new int();		// ��ֵΪ 0
	int* p2 = new int(100);		// ��ֵ���
}

void test1_2() {
	// (1.2) new �� delete �Ļ�������ԭ��
	// new ��������һ���ؼ���/��������������һ������
	// new �Ⱥ�ֱ������ operator new() �͹��캯�� A::A()
	// �� operator new ��һ�������ĺ��������ڲ������� malloc()
	_nmsp1::A* pa = new _nmsp1::A;

	// delete ��������һ���ؼ���/��������������һ������
	// delete �Ⱥ�ֱ��������������A::~A()�� operator delete()
	// �� operator delete() ��һ�������ĺ��������ڲ������� free()
	delete pa;
}

int main() {
	// һ��������ع�
	// (1.1) new T �� new T() ������
 	// test1_1();

	// (1.2) new �� delete �Ļ�������ԭ��
	test1_2();
	
	return 0;
}