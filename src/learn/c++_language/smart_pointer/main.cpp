// #include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace _nmsp1 {
	class A {
	public:
		A() {
			std::cout << "A::A() ִ�У�" << std::endl;
		}
		~A() {
			std::cout << "A::~A() ִ�У�" << std::endl;
		}
	};

	std::shared_ptr<int> make_int(int value) {
		return std::shared_ptr<int>(new int(value));
	}

	void func_value(std::shared_ptr<int> tv) {

	}

	void func_reference(std::shared_ptr<int>& tv) {

	}

	std::shared_ptr<int> func_return_value(std::shared_ptr<int>& tv) {
		return tv;
	}
}

namespace _nmsp2 {
	// ���ͶѶ���ɾ����
	void deleter(int* p) {
		std::cout << "���� deleter ִ�У�" << std::endl;
		// ���ѡ��ȡ��ϵͳȱʡ��ɾ���������������Լ�ɾ������
		delete p;
	}
	
	// ��װ shared_ptr ����
	template<typename T>
	std::shared_ptr<T> make_shared_array(size_t size) {
		return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
	}

	void func(std::shared_ptr<int> p) {

	}
}

namespace _nmsp3 {
	class A {
	public:
		std::shared_ptr<A> get_self() {
			return std::shared_ptr<A>(this);
		}
		// this ָ���൱��ָ��������ָ��
		A* get_this() {
			return this;
		}
	};

	class SA : public std::enable_shared_from_this<SA>
	{
	public:
		std::shared_ptr<SA> get_self() {
			return shared_from_this();
		}
	};


	class CB;
	class CA {
	public:
		std::weak_ptr<CB> m_b;
		~CA(){
			std::cout << "CA::~CA() ִ�У�" << std::endl;
		}
	};

	class CB {
	public:
		std::shared_ptr<CA> m_a;
		~CB() {
			std::cout << "CB::~CB() ִ�У�" << std::endl;
		}
	};
	
	auto func() {
		return std::unique_ptr<std::string>(new std::string("Hello!"));
	}

	auto return_unique() {
		// ����һ���ֲ� unique_ptr ���󣨽���ֵ��
		// һ�������ϵͳ������һ����ʱ�Ķ��������أ�������ƶ����캯���Ļ���������ƶ����캯����
		return std::unique_ptr<std::string>(new std::string("Hello!"));
	}
}

namespace _nmsp4 {
	void deleter_function(std::string*& p) {
		std::cout << "delter_function() ִ�У�" << std::endl;
		delete p;
		p = nullptr;
	}
	
	class deleter_class {
	public:
		void operator()(std::string*& p) {
			std::cout << "delter_class() ִ�У�" << std::endl;
			delete p;
			p = nullptr;
		}
	};
}

void test1() {
	// һ��ֱ��ʹ�� new / delete �����ڴ�
	int* p1 = new int;
	// ֵ��ʼ����ʹ�� () �� {} ���г�ʼ��
	// �����������ͣ�ʹ��ֵ��ʼ�� new �������ڴ��ֵΪ 0
	int* p2 = new int();
	int* p3 = new int{};
	int* p4 = new int{ 100 };

	// ���������new ʵ�ʵ��õ��ǹ��캯��
	std::string* p5 = new std::string();
	std::string* p6 = new std::string("abc");
	std::string* p7 = new std::string(5, 'a');
	std::vector<int>* p8 = new std::vector<int>{ 1,2,3,4,5 };

	// ���������ֵ��ʼ�����õ�Ĭ�Ϲ��캯��
	_nmsp1::A* p9 = new _nmsp1::A;
	_nmsp1::A* p10 = new _nmsp1::A();

	// ָ������ָ��
	const int* cp1 = new const int(100);
	// ָ�����ĳ���ָ��
	const int* const cp2 = new const int(200);
	cp1 = new const int(300);
	// cp2 ��ָ�����ĳ�ָ�벻�ܱ���ֵ
	// cp2 = new const int(400);

	// ע�⣬delete �� p2 ��Ұָ�룬ָ��δ֪�ڴ棬������ nullptr
	delete p2;
	// ����� delete �� p2 ��ֵΪ��ָ��
	p2 = nullptr;

	int* tp1 = new int(100);
	int* tp2 = tp1;
	// delete tp1 ��tp1 ָ��δ֪�ڴ棬 tp2����ָ��ԭ�ڴ棬�������е�ֵ�����δֵ֪
	delete tp1;

	// �ܽ�
	// (1) new �����ڴ�û�� delete �ᵼ���ڴ�й¶���ڴ�й¶��һ���̶Ȼᵼ�³������
	// (2) delete ����ڴ治����ʹ�ã�����ᷢ���쳣
	// (3) delete ĳ��ָ��󣬽�ָ����Ϊ nullptr
	// (4) ����ͬʱ delete һ���ڴ� 2 �� 
}

void test2() {
	// ����new / delete ̽��������ָ�������shared_ptr ����
	// (2.1) new / delete ̽��
	// sizeof �ǣ��ؼ��� / �������������һ������
	// malloc, free ��Ҫ���� C ���ԣ���Ϊ malloc ���ܵ�����Ĺ��캯�����ɶ���
	// new �� delete ��Ҫ���� C++
	// new ���������ڴ棬������һ�¶����ʼ��������delete �����ͷ��ڴ棬������һЩ�����������
	
	// new ���ù��캯�����Զ��Ϸ���Ķ�����г�ʼ��
	_nmsp1::A* p = new _nmsp1::A();
	// new �������������������ϵĶ����������ͷ��ڴ�
	delete p;

	// (2.2) operator new() �� operator delete()
	// new �ȷ����ڴ棨ͨ�� operator new() �������ڴ棩��Ȼ�������Ĺ��캯������ʼ���ڴ�
	// void* operator new(size_t _Size) ���ڷ���ԭʼ�ڴ�
	void* ptr = operator new(100);
	// delete �ȵ��������������ͷ��ڴ棨ͨ�� operator delete() ���ͷ��ڴ棩
	// void operator delete(void* _Block) �����ͷŷ�����ڴ�
	operator delete(ptr);

	// (2.3) ���� new ��μ�¼������ڴ��С�� delete ʹ��
	// ��ͬ������ new �ڲ�ʵ�ַ�ʽ��ͬ
	int* p1 = new int; 
	// delete ���֪����Ҫ���յ��ڴ�Ĵ�С��new �ڲ��м�¼���ƣ�
	delete p1;

	// (2.4) ������ͷ�һ������
	int* p2 = new int(100);
	// int* p3 = new int[2]; ���𣿣�����
	// �������Ͳ���Ҫ��¼ new ���� int ����
	int* p3 = new int[2]();
	// ���� 1 �ι��캯��
	_nmsp1::A* p4 = new _nmsp1::A();
	// �� A ����� 4 �ֽ� 
	// _nmsp1::A* p5 = new _nmsp1::A[2]; ���𣿣�����
	// ���� 2 �ι��캯��
	_nmsp1::A* p5 = new _nmsp1::A[2]();

	delete p2;
	delete[] p3;
	// ���� 1 ����������
	delete p4;
	// ���� 2 ����������������� 4 �ֽ����ڼ�¼ new �Ķ������
	delete[] p5;

	// (2.5) Ϊʲô new / delete ��new[] / delete[] Ҫ���ʹ��
	// ���������� int ����Ҫ���ù��캯�������� new[] ʱ��ϵͳû�з������� 4 �ֽ�
	// ���Զ��� int ���ͣ����� new[]������ delete �� delete[] ��һ��

	// ����һ������ʹ�� new[] �������ڴ棬ȴʹ�õ����� delete�������� delete[]�����ͷ��ڴ棬��Ҫ���������ĳ������
	// 1) �������������������
	// 2) ����������������͵�û���Զ����������������Ϊ new[] ʱû�ж������� 4 �ֽ����ڼ�¼���������������

	// Ϊʲô���ṩ�Լ����������������� delete[] �ͷ��ڴ�ᵼ���쳣
	// 36 �� 1 Сʱ�̸����ڴ��ͷ�
	// ˼������������������������ �����о� operator new �� operator delete ��ʵ��
}

void test3_1() {
	// new delete ������
	// a) delete ǰ�׳��쳣�����ߴ�����֧���������� delete
	// b) ���ָ��ָ��ͬһ���ڴ浼�µ�Эͬ����

	// ����ָ��� "��ָ��" ���а�װ���ܹ��Զ��ͷ� new �������ڴ�
	// (3.1.1) ��׼���� 4 ������ָ�������
	// auto_ptr, unique_ptr, shared_ptr, weak_ptr
	// ��������̬�����ڴ������������ڣ���Ч��ֹ�ڴ�й¶
	
	// shared_ptr������ʽָ�루�����������Ȩ�������ָ��ָ��ͬһ���������һ��ָ�뱻����ʱ������Żᱻ�ͷ�
	// weak_ptr�����ڸ��� shared_ptr
	// unique_ptr����ռʽָ�루��ռ��������Ȩ����ֻ��һ��ָ���ܹ�ָ����󣬸ö��������Ȩ�����ƽ���move��

	// shared_ptr: �������Դ������������ڲ��������ü������������ߵĸ����������һ�������߱�����ʱ����Դ�����ͷš�

	// (3.1.2) shared_ptr ����
	// shared_ptr �ж��⿪���������ڶ��ָ�빲��һ���Ѷ���
	// shared_ptr �Ĺ���ԭ�������ü�����ֻ�����һ��ָ��Ѷ����ָ�뱻����ʱ������Żᱻ�����ͷ�
	// �������һ��ָ��Ѷ���� shared_ptr ָ�룬�����������������ʱ���������ͷ�ָ��ĶѶ���
	// a) shared_ptr ָ�뱻������ʱ��
	// b) shared_ptr ָ�����������ʱ��

	// shared_ptr �ļ�ʹ��
	
	// ָ�� int �����ڴ������ָ�룬������Ϊ sp����ĿǰΪ�գ�û��ָ���κζ���
	std::shared_ptr<int> sp;

	// ʹ�� new ��乹�� shared_ptr
	std::shared_ptr<int> sp1(new int(100));

	// ����ʹ�õȺŸ�ֵ����Ϊ����ָ�벻�ܽ�����ʽ����ת����ֻ�ܽ�����ʽ����ת��������ֱ�ӹ��캯����ʼ��
	// std::shared_ptr<int> sp2 = new int(100);
	
	// ʹ����ʽǿ������ת��
	std::shared_ptr<int> sp2 = static_cast<std::shared_ptr<int>>(new int(100));

	// return shared_ptr
	std::shared_ptr<int> sp3 = _nmsp1::make_int(100);

	// ��������ָ���ʼ�� shared_ptr 
	// ������ʹ����ָ��� shared_ptr ���׳���������Ҫ������ָ��� shared_ptr
	int* p2 = new int(100);
	std::shared_ptr<int> sp4(p2);

	// (3.1.3) make_shared
	// make_shared ����ģ�壬�ܰ�ȫ��Ч�Ĵ��� shared_ptr ����
	std::shared_ptr<int> sp5 = std::make_shared<int>(100);
	std::shared_ptr<std::string> sp6 = std::make_shared<std::string>(5, 'a');
	std::shared_ptr<int> sp7 = std::make_shared<int>();
	sp7 = std::make_shared<int>(400);
	auto sp8 = std::make_shared<std::string>(6, 'a');
}

void test3_2() {
	// (3.2.1) shared_ptr ���ü��������Ӻͼ���
	// ���ü�������
	// ÿ�� shared_ptr �����¼�ж��ٸ� shared_ptr ͬʱָ����ͬ�ĶѶ���
	//// ����1 ���������������� ����ط���¼���ü�����Ŀ���������Լ�������ÿ�� shared_ptr ������һ�����ü����ı�����
	//// ����2 ���������������������ü�������ʱ������ԭ����ʲô
	
	// ʲô����£�����ָ��ͬһ���Ѷ���� shared_ptr ָ������ü����������� 1
	// a) ����������ָ����п���ʱ
	auto sp1 = std::make_shared<int>(100);
	auto sp2 = sp1;

	// b) ������ָ����Ϊʵ��ֵ���ݷ����˶��󿽱�ʱ
	_nmsp1::func_value(sp1);
	// �� shared_ptr �����ã����ü�������� 1
	_nmsp1::func_reference(sp1);

	// c) return shared_ptr �����˿���ʱ
	auto sp3 = _nmsp1::func_return_value(sp1);

	// ���ü�������
	// ʲô����£�����ָ��ͬһ���Ѷ���� shared_ptr ָ������ü���������� 1
	// a) �� shared_ptr ������ֵ������ָ��һ���µĶѶ���
	sp3 = std::make_shared<int>(100);
	sp2 = std::make_shared<int>(200);
	sp1 = std::make_shared<int>(300);

	// b) �ֲ��� shared_ptr �뿪��������
	_nmsp1::func_value(sp1);
	
}

void test3_3() {
	// (3.3) shared_ptr ���ò���
	// use_count()�������ж��ٸ�����ָ��ָ��ͬһ���Ѷ�����Ҫ���ڵ���Ŀ��
	// unique()���жϸ�����ָ���Ƿ��ռһ���Ѷ������ shared_ptr ��ʼ��ʱδָ��Ѷ����򷵻� false)
	std::shared_ptr<int> sp1(new int(100));
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// ����unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	auto sp2 = sp1;
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// ����unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	auto sp3 = sp2;
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// ����unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	std::cout << std::endl;

	// reset()���ָ� / ��λ / ����
	// a) �� reset() ��������ʱ��
	// �� sp ��Ψһָ��öѶ����ָ�룬��ô�ͷ� pi ��ָ��ĶѶ��󣬲��� pi �ÿ�
	// �� sp ����Ψһָ��öѶ����ָ�룬���� share_ptr ָ������ü����� 1��ͬʱ�� pi �ÿ�
	std::shared_ptr<int> p1(new int(100));
	auto p2 = p1;
	p2.reset();
	std::cout << "use_count = " << p1.use_count() << std::endl;
	p1.reset();
	std::cout << "use_count = " << p1.use_count() << std::endl;
	std::cout << std::endl;
	// reset()���ָ� / ��λ / ����
	// a) �� reset() ��������ʱ��
	// �� sp ��Ψһָ��öѶ����ָ�룬��ô�ͷ� pi ��ָ��ĶѶ��󣬲��� pi �ÿ�
	// �� sp ����Ψһָ��öѶ����ָ�룬���� share_ptr ָ������ü����� 1��ͬʱ�� pi �ÿ�
	std::shared_ptr<int> p3 = std::make_shared<int>(100);
	p3.reset(new int(300));
	std::cout << "*p3 = " << *p3 << std::endl;
	// �� shared_ptr �������ͨ�� reset() ���³�ʼ��
	std::shared_ptr<int> p4;
	// δ��ʼ��ֱ�ӵ��� *p4 ������쳣
	// std::cout << "*p4 = " << *p4 << std::endl;
	p4.reset(new int(400));
	std::cout << "*p4 = " << *p4 << std::endl;
	std::cout << std::endl;

	// *p �����ã���� p ָ��Ķ���
	std::shared_ptr<int> pt1(new int(123456));
	std::cout << "*pt1 = " << *pt1 << std::endl;
	std::cout << std::endl;

	// get()����������ָ����������������ָ��
	// ��ҪС��ʹ�ã��������ָ����� reset() �ͷ�����ָ��ĶѶ�����ô������ص���ָ��Ҳ��û��������
	// get() ��ʹ���ǲ���ȫ�ģ�����׼��ʵ�� get() �����ǿ��ǵ���Щ����ֻ�ܴ������õ���ָ�룬���� Linux �µ�ϵͳ���ã����������
	// ��Ҳ��ʾ�˳���Ա����Ȼ����ָ����������ڴ����������ָ��Ҳ�������ܵģ�������Ҫ���ڴ�������������;���
	std::shared_ptr<int> pt2(new int(654321));
	std::cout << "*pt2 = " << *pt2 << std::endl;
	int* p = pt2.get();
	*p = 45;
	std::cout << "*pt2 = " << *pt2 << std::endl;
	std::cout << std::endl;

	// swap()�����ڽ��� 2 ������ָ����ָ��ĶѶ���һ�㲻����
	std::shared_ptr<std::string> ps1(new std::string("123"));
	std::shared_ptr<std::string> ps2(new std::string("abc"));
	std::cout << "*ps1 = " << *ps1 << std::endl;
	std::cout << "*ps2 = " << *ps2 << std::endl;
	// ps1.swap(ps2);
	std::swap(ps1, ps2);
	std::cout << " After Swap ps1 and ps2!" << std::endl;
	std::cout << "*ps1 = " << *ps1 << std::endl;
	std::cout << "*ps2 = " << *ps2 << std::endl;
	std::cout << std::endl;

	// = nullptr
	// a) ����ָ��ĶѶ������ü�����һ�������ü�����Ϊ 0 �ˣ����ͷ�����ָ����ָ��Ķ���
	// b) ������ָ���ÿ�
	std::shared_ptr<std::string> ps3(new std::string("nullptr"));
	auto ps4 = ps3;
	ps4 = nullptr;
	ps3 = nullptr;
	std::cout << std::endl;

	// ����ָ��������Ϊ�ж�����
	std::shared_ptr<std::string> ps5(new std::string("Hello!"));
	if (ps5) std::cout << "*ps5 = " << *ps5 << std::endl;
	ps5 = nullptr;
	if (!ps5) std::cout << "*ps5 is empty!" << std::endl;
	std::cout << std::endl;
}

void test3_4() {
	// (3.4.1) ָ��ɾ����
	// ����ָ���ܹ��ں��ʵ�ʱ��ɾ����ָ��ĶѶ���
	// ȱʡ����£�����ָ�뽫 delete ��ΪĬ�ϵ���Դ������ʽ
	// ���ϣ����һЩ����Ĵ��������ӡ��־�ȣ����Ը�����ָ��ָ��ɾ�����ԴﵽĿ��
	// ָ��ɾ�����ķ���һ�����ڲ��������ɾ�����ĺ�����
	std::shared_ptr<int> pi(new int(123456), _nmsp2::deleter);
	pi.reset();
	std::cout << std::endl;

	// ʹ�� lambda ���ʽ��Ϊɾ����
	std::shared_ptr<int> pi1(new int(123456), [](int* p)->void {
		std::cout << "���� lambda deleter ִ�У�" << std::endl;
		// ���ѡ��ȡ��ϵͳȱʡ��ɾ���������������Լ�ɾ������
		delete p;
	});
	pi1.reset();
	std::cout << std::endl;

	// (3.4.2) share_ptr ����̬���������
	// ��Щ���Ĭ��ɾ���������ˣ���Ҫ�ṩ�Զ���ɾ����
	// һ�����͵��������� share_ptr ����̬����ʱ
	// share_ptr �������ζ�̬����
	std::shared_ptr<int> pi2(new int[10]{ 0 }, [](int* p)->void {
		std::cout << "�������� lambda deleter ִ�У�" << std::endl;
		delete[] p;
	});
	pi2.reset();
	std::cout << std::endl;

	// share_ptr ���������Ͷ�̬����
	std::shared_ptr<_nmsp1::A> pa(new _nmsp1::A[10](), [](_nmsp1::A* p)->void {
		std::cout << "���������� lambda deleter ִ�У�" << std::endl;
		delete[] p;
	});
	pa.reset();
	std::cout << std::endl;
	
	// ʹ�� default_delete ����ɾ������default_delete �Ǳ�׼���е�ģ���ࣩ
	std::shared_ptr<int> pi3(new int[10]{ 0 }, std::default_delete<int[]>());
	pi3.reset();
	std::cout << std::endl;
	
	std::shared_ptr<_nmsp1::A> pa1(new _nmsp1::A[10](), std::default_delete<_nmsp1::A[]>());
	pa1.reset();
	std::cout << std::endl;

	// C++17 ��׼֧�ֵ����﷨
	std::shared_ptr<int[]> pi4(new int[10]{ 0 });
	pi4.reset();
	std::cout << std::endl;

	std::shared_ptr<_nmsp1::A[]> pa2(new _nmsp1::A[10]());
	pa2.reset();
	std::cout << std::endl;

	// shared_ptr ʹ���±���ʷ�
	std::shared_ptr<int[]> p(new int[10]{ 0 });
	for (int i = 0; i < 10; ++i) {
		p[i] = i + 1;
		std::cout << p[i];
	}

	// ʹ�÷�װ�� shared_ptr ����
	auto array_p1 = _nmsp2::make_shared_array<int>(10);
	auto array_p2 = _nmsp2::make_shared_array<_nmsp1::A>(10);

	// ָ��ɾ��������˵��
	// ��ʹ 2 �� shared_ptr ָ���˲�ͬ��ɾ������ֻҪ������ָ��Ķ���������ͬ
	// ��ô������ shared_ptr ����ͬһ����
	auto lambda1 = [](int* p)->void {
		std::cout << "lambad1 ִ�У�\n";
		delete p;
	};
	auto lambda2 = [](int* p)->void {
		std::cout << "lambad2 ִ�У�\n";
		delete p;
	};
	
	// �ڴ���ʱ��������ڴ��� lambda1 �ͷţ��������ڸ�ֵ��ԭ���¸���ɾ����
	std::shared_ptr<int> sp1(new int(100), lambda1);
	std::shared_ptr<int> sp2(new int(200), lambda2);
	// sp2 ���ȵ��� lambda2 ���Լ���ִ�еĶѶ����ͷŵ���Ȼ��ָ�� sp1 ��ָ��ĶѶ���, sp1 �����ü���Ϊ 2
	// ���� test3_4() ִ�н���������� lambda1 ���ͷ� sp1 ��ָ��ĶѶ���
	// ���д���Ҳ��ʾ sp1 �� sp2 ����ͬһ�����ͣ����Բ��ܽ� sp1 ��ֵ�� sp2
	sp2 = sp1;

	// ������ͬ����ζ�ſ��Խ� sp1 �� sp2 ������ӦԪ�����͵�������
	std::vector<std::shared_ptr<int>> v{ std::move(sp1), sp2 };

	// ��Ҫע�⣬make_shared ����ָ��ɾ����
}

void test4_1() {
	// weak_ptr: �� shared_ptr ���ʹ�ã���Ȼ�ܷ�����Դ��ȴ��������Դ������Ȩ����Ӱ����Դ�����ü���
	// �п�����Դ�ѱ��ͷţ��� weak_ptr ��Ȼ���ڣ����ÿ�η�����Դʱ����Ҫ�ж���Դ�Ƿ���Ч


	// (4.1.1) weak_ptr ��������ָ�룬�����������ã�
	// weak_ptr ���ڸ��� shared_ptr����ʾ������ָ�룬��֮��Ե� shared_ptr ��ʾǿ����ָ��
	// weak_ptr ����ָ��һ���� shared_ptr ����ĶѶ��󣬵� weak_ptr ���������ָ��Ѷ������������
	// ������֮���� weak_ptr ָ��һ���� shared_ptr ����ĶѶ���ʱ������Ӱ�������ü���
	// Ҳ����˵��weak_ptr �Ĺ���������������ӻ������ָ���������ü���
	// ���Ѷ�������ü�����Ϊ 0 ʱ�������Ƿ��� weak_ptr ָ��ö��󣬶�����Ӱ�� shared_ptr �����ͷŸö���
	/////// �ɴ˿��ܵ��� weak_ptr ָ��Ķ����Ѿ��� shared_ptr �����ˣ����� weak_ptr ʹ�ö���������
	// weak_ptr ���Խ�Ϊ���ڼ��� shared_ptr ָ�������������ڣ��Ƕ� shared_ptr ����������
	// weak_ptr ����һ�ֶ���������ָ�룬��������������ָ�����Դ������һ���Թ��ߣ�
	// weak_ptr �ܹ���������ָ��Ķ����Ƿ����

	// (4.1.2) ���� weak_ptr
	auto pi = std::make_shared<int>(100);
	// �˴�������ʽ����ת��
	std::weak_ptr<int> pi1 = pi;
	std::weak_ptr<int> pi2 = pi1;
	
	// ���� weak_ptr ��ָ��Ķ�����ܱ��ͷţ����Բ���ʹ�� weak_ptr ֱ�ӷ��ʶ��󣬶�������� lock() ��������
	// lock()�����ڼ�� weak_ptr ��ָ��Ķ����Ƿ����
	// ������ڣ��򷵻�һ��ָ��ö���� shared_ptr��ͬʱָ��ö�������ü����� 1
	// ��������ڣ��򷵻�һ���յ� shared_ptr
	auto sp = std::make_shared<std::string>("Hello!");
	std::weak_ptr<std::string> sp1(sp);
	auto sp2 = sp1.lock();
	if (sp2) std::cout << *sp2 << std::endl;
	sp.reset();
	sp2.reset();
	// sp1 ���ӵ���ָ��Ķ����Ѿ���������
	auto sp4 = sp1.lock();
	if (!sp4) std::cout << "Not Exist!" << std::endl;
}

void test4_2() {
	// (4.2) weak_ptr ���ò���	

	// use_count()����ȡ����ָ�빲���������� shard_ptr ��������ǿ���õ�������
	auto ps = std::make_shared<int>(100);
	auto ps1 = ps;
	std::weak_ptr<int> pw = ps;
	std::cout << "pw.use_count(): "<<pw.use_count() << std::endl;

	 // expired()���ж� weak_ptr ָ��ĶѶ����Ƿ���ڣ�ǿ���ü����Ƿ��Ϊ 0��
	std::cout << "pw.expired(): " << (pw.expired() ? "True" : "False") << std::endl;
	ps.reset();
	ps1.reset();
	std::cout << "pw.expired(): " << (pw.expired() ? "True" : "False") << std::endl;

	// reset()���� weak_ptr �ÿ�
	// ����Ӱ��ָ��ö����ǿ������������ָ��ö����������������һ
	ps = std::make_shared<int>(100);
	pw = ps;
	auto pw1 = pw;

	// lock()�����ڼ�� weak_ptr ��ָ��Ķ����Ƿ����
	// ������ڣ��򷵻�һ��ָ��ö���� shared_ptr��ͬʱָ��ö�������ü����� 1
	// ��������ڣ��򷵻�һ���յ� shared_ptr
	
	auto ps2 = std::make_shared<int>(100);
	std::cout << "ps2: " << *ps2 << std::endl;
	std::weak_ptr<int> pw2 = ps2;
	// ps2.reset();
	if (!pw2.expired()) {
		auto tmp = pw2.lock();
		if (tmp) *tmp = 200;
		std::cout << "ps2: " << *ps2 << std::endl;
	}else {
		std::cout << "ps2 expired!" << std::endl;
	}
	
	// weak_ptr �ĳߴ�����
	// weak_ptr �ĳߴ�� shared_ptr �ߴ�һ����
	int* p3;
	std::weak_ptr<int> pw3;
	// 4 �ֽڣ���ָ��Ĵ�С��
	std::cout << "sizeof(p3) = " << sizeof(p3) << std::endl;
	// 8 �ֽڣ����� 2 ����ָ�룩
	// ��һ��ָ��ָ����ڴ��Ϸ���Ķ��� �ڶ���ָ��ָ��ö�����ƿ���׵�ַ
	// ���ƿ�ʵ�������ڴ�����һ�� shared_ptr ʱ�����ģ�������һ�� weak_ptr ʱֻ��ָ����������������
	// ���ƿ��а�����ָ��ö����ǿ���ü����������ü����Լ��������ݣ����Զ���ɾ�����ȣ�
	std::cout << "sizeof(pw3) = " << sizeof(pw3) << std::endl;
}

void test5_1() {
	// (5.1) ������ָ��� get() ����
	// a) ������ָ��
	int* p = new int(100);
	// ���ܽ�����ʽ����ת��
	// _nmsp2::func(p);
	_nmsp2::func(std::shared_ptr<int>(p));
	// �˴� p ָ����ڴ�ռ��ڴ��ݽ��� func() ���ͷ��ˣ����²�������ʹ�� p ָ��
	// *p = 200;

	// ��ȷ�÷�
	// ������ָ���ʼ������ָ����ڴ����Ȩ���ƽ����� shared_ptr ����
	// �ں��������о�����Ҫ��ʹ����ָ�룬������ʱ��ͷŵ��ڴ�
	int* p1 = new int(200);
	std::shared_ptr<int> sp1(p1);
	_nmsp2::func(sp1);
	*p1 = 300;

	// b) ���Բ�����ͬһ����ָ���ʼ����� shared_ptr ����
	// ����ظ���ʼ�����ᵼ����Щ shared_ptr ��������ʱ����ͷ�ͬһ���ڴ棬�����쳣
	// int *p2 = new int(300);
	// std::shared_ptr<int> sp2(p2);
	// std::shared_ptr<int> sp3(p2);
	
	// c) ���� get() ����ָ��
	// 1) ʹ�� get() ������ָ�� p ʱ���п���ʹ�ñ��ͷŵ��ڴ�
	// ������ʹ�� p ʱ������ shared_ptr �����ڶ�������������
	// int* p3 = nullptr;
	// {
	//	 std::shared_ptr<int> sp4(new int(100));
	// 	 p3 = sp4.get();
	// }
	// *p3 = 200;
	
	
	// 2) ʹ�� get() ������ָ�� p ʱ�����ܶ� p ʹ�� delete
	// ��Ϊ�ڴ�ʵ���� shared_ptr ������Ӧ���Լ�����
	// std::shared_ptr<int> sp5(new int(100));
	// int* p4 = sp5.get();
	// delete p4;

	// 3) ������ get() ���ص���ָ��ȥ��ʼ����������ָ��
	std::shared_ptr<int> sp6(new int (100));
	{
		std::shared_ptr<int> sp7(sp6.get());
	}
	// ���д�����ͷŵ��ڴ渳ֵ��ȴ��ȫ���ᱨ��
	// �൱�ڷǷ�ʹ���˲����ڱ�����Ķ��ڴ棬��ᵼ�²���Ԥ�ϵĺ��
	// �˴� delete �����ָ�붼���ܸ�ֵ��������ָ��ȴ���ԣ���һ���ǳ����޵�����
	*sp6 = 5;
	// �����ܹ�������ӡ����
	std::cout << *sp6 << std::endl;
	
	//// ���Զ��ͷ��ڴ�����ָͨ�븳ֵ
	//int* p5 = new int(200);
	//delete p5;
	//// ����д����Ȩ�޳�ͻ���쳣
	//*p5 = 100;
	//std::cout << *p5 << std::endl;
}

void test5_2() {
	// (5.2.1) ��Ҫ�������ָ�� this ��Ϊ shared_ptr ����
	std::shared_ptr<_nmsp3::A> pa1(new _nmsp3::A());
	// �൱����ָ��������ָ��ȥ��ʼ��һ������ָ�룬����һ���ڴ����ͷ�
	// std::shared_ptr<_nmsp3::A> pa2 = pa1->get_self();
	// std::shared_ptr<_nmsp3::A> pa3(pa1->get_this());

	// ʹ�� enable_shared_from_this ��
	std::shared_ptr<_nmsp3::SA> pa4(new _nmsp3::SA());
	std::shared_ptr<_nmsp3::SA> pa5 = pa4->get_self();
	// enable_shared_from_this ��һ�� weak_ptr �ܹ����� this
	// �ڵ��� shared_from_this �������ʱ��ʵ�����ڲ������� weak_ptr ��lock()���� �����ɣ���
	// �벻̫��ʵ�ֵ�ԭ�� 
	
	// ˼��������һ����ָ���ʼ�� weak_ptr ��
	// ʵ��֤������
	//_nmsp3::SA* p1 = new _nmsp3::SA();
	//std::weak_ptr<_nmsp3::SA> pw1;
	//pw1.reset(p1);

	// (5.2.2) ����ѭ������
	//std::shared_ptr<_nmsp3::CA> pca(new _nmsp3::CA());
	//std::shared_ptr<_nmsp3::CB> pcb(new _nmsp3::CB());
	//// ѭ�����õ��� 2 ������û�ͷţ�Ϊʲô����
	//// ����˼��ԭ��
	//// ���� pcb ������������ʱ���������� CB ��������ü�������δ�����ڴ��ϵ� CB ����
	//// CB ����ȴ� CA �ĳ�Ա���� m_b �������� CB �����ü�����Ϊ 0��ʱ���������ͷ��ڴ�
	//// �� pca ����ʱ��Ҳֻ�ǽ� CA ��������ü�����һ����δ�����ͷ��ڴ�
	//// ����CA ����ȴ� CB �ĳ�Ա���� m_a �������� CA �����ü�����Ϊ 0��ʱ���������ͷ��ڴ�
	//// �ɴ� CA��CB �໥���ڵȶԷ���ָ���Լ��� shared_ptr ��Ա�����������������߶����������������ڴ�й¶
	//pca->m_b = pcb;
	//pcb->m_a = pca;

	// ��������ǽ� CA��CB ���κ�һ�� shared_ptr �ĳ� weak_ptr
	// ������һ����� share_ptr ��Ϊ weak_ptr ��ʹ������౻�����ͷţ��൱�ڴ������໥�ĵȴ�
	// ֻҪ�ܱ�֤ԭ�� share_ptr ���������ڣ�weak_ptr ��ȫ�ܹ����� shared_ptr ʹ�ã���������Ҫ��һЩ
	std::shared_ptr<_nmsp3::CA> pca(new _nmsp3::CA());
	std::shared_ptr<_nmsp3::CB> pcb(new _nmsp3::CB());
	pca->m_b = pcb;
	pcb->m_a = pca;
}

void test5_3(){
	// (5.3.1) ����˵��
	// shared_ptr ռ�ÿռ�����ͨ��ָ��� 2 ����С
	int* p;
	std::shared_ptr<int> ps;
	// 4 �ֽڣ���ָ��Ĵ�С��
	std::cout << "sizeof(p3) = " << sizeof(p) << std::endl;
	// 8 �ֽڣ����� 2 ����ָ�룩
	// ��һ��ָ��ָ����ڴ��Ϸ���Ķ��� �ڶ���ָ��ָ��ö�����ƿ���׵�ַ
	// ���ƿ�ʵ�������ڴ�����һ��ָ��Ѷ���� shared_ptr ʱ���������˺������� shared_ptr �������ڴ����ˣ����� �Ҳ²�ģ�����ȷ����
	// ���ƿ��а�����ָ��ö����ǿ���ü����������ü����Լ��������ݣ����Զ���ɾ�������������ȣ�
	// ���� weak_ptr Ҳָ�� share_ptr �����Ŀ��ƿ飬���� weak_ptr ���������� share_ptr ������ 
	std::cout << "sizeof(pw3) = " << sizeof(ps) << std::endl;

	// ���ƿ鴴����ʱ��
	// a) make_shared() ���䲢��ʼ��һ�����󣬲�����ָ��ö���� shared_ptr
	// ���� make_shared() ���ǻᴴ��һ���µĿ��ƿ�

	// b) �� new ����ָ���ʼ��һ���µ� shared_ptr ʱ
	// ���ʹ��ͬһ����ָ���ʼ����� share_ptr����ᴴ��������ƿ飬������ͷ�ͬһ���ڴ棬�����쳣
	
	// (5.3.2) �ƶ�����
	std::shared_ptr<int> sp1(new int(100));
	// �ƶ����� sp2��sp1 ������ָ��ԭ�������ü�����Ϊ 1
	// �൱�� sp1 ���Ѷ���Ĺ������Ȩ�ƽ��������ǹ������� sp2
	std::shared_ptr<int> sp2 = std::move(sp2);
	
	// (5.3.3) ����˵����ʹ�ý���
	// a) ����Ϊ����ָ���ṩ������
	// b) �Դ��ڴ���ȻҪС�Ľ���
	// c) ���ȿ���ʹ�� make_shared() Ч�ʸ���
	// ��˵���ܷ��� 2 ���ڴ棬1 ��Ϊ string ���䣬1 ��Ϊ���ƿ����
	std::shared_ptr<std::string> ps1(new std::string("abc"));
	// ��˵ֻ�÷��� 1 ���ڴ棬һ����Ϊ string �Ϳ��ƿ�ͬʱ����һ����ڴ�
	std::shared_ptr<std::string> ps2 = std::make_shared<std::string>("abc");
}

void test6_1() {
	// (6.1.1) unique_ptr �����볣�ò���(1)
	// unique_ptr �Ƕ�ռʽ��ר������Ȩ������ָ��
	// Ҳ����˵ͬһʱ�̣�ֻ����һ�� unique_ptr ָ��Ѷ���
	
	// (6.1.2) unique_ptr �ĳ�ʼ��
	// ��ʼ��һ������ָ�� int ����Ŀ�����ָ��
	std::unique_ptr<int> pi;
	if (!pi)std::cout << "pi is empty!" << std::endl;
	std::unique_ptr<int> pi1(new int(100));
	if (!pi)std::cout << "*pi1 = " <<*pi1<< std::endl;

	// make_unique() (C++14)
	// make_unique �� make_shared һ������֧��ָ��ɾ����
	// ��������ʹ�� make_unique�����ܸ��ߣ��������Ϳ��ܲ�ռ���ƣ���������ռ���ƣ�Ч����Ҫ�����ڴ���䷽�棩
	std::unique_ptr<std::string> ps = std::make_unique<std::string>("Hello!");

	// (6.1.3) unique_ptr ���ò���(1)
	// a) unique_ptr ��֧�ֿ������캯���͸�ֵ�����
	// ���ڶ�ռ�������� unique_ptr ��֧�ֿ������캯���͸�ֵ���������֧�� move ����
	std::unique_ptr<int> pi2(new int(100));
	// ����ʹ����ɾ���ĺ���
	// std::unique_ptr<int> pi3(pi2);
	// ����ʹ����ɾ���ĺ���
	// std::unique_ptr<int> pi4 = pi2;
	std::unique_ptr<int> pi5 = std::move(pi2);
			
	// b) realse()�������ԶѶ���Ŀ���Ȩ��������ָ���ÿգ���������ָ��
	// ���ڷ��ص���ָ�룬�����ֶ� delete��Ҳ���������ʼ����һ������ָ��
	std::unique_ptr<int> up1(new int(0));
	std::unique_ptr<int> up2(up1.release());
	if (!up1)std::cout << "up1 is empty!" << std::endl;
	// ע�⣺ֻ���� release() ����ȥ���ղ� delete ���ص���ָ��ᵼ���ڴ�й¶
	// up2.release();
	std::unique_ptr<int> up3;
	auto p1 = up1.release();
	auto p2 = up3.release();
	// ͨ����֤�����ڳ�ʼ��Ϊ�ջ� release() ��� unique_ptr�����ڲ�����ָ�붼��Ϊ�� nullptr
	if (p1) std::cout << "p1 ָ��δ֪�ڴ�!" << std::endl;
	else std::cout << "p1 is nullptr!" << std::endl;
	if (p2) std::cout << "p2 is ָ��δ֪�ڴ棡" << std::endl;
	else std::cout << "p2 is nullptr!" << std::endl;
	// ͨ����֤��delete �����ָ��ָ��δ֪�ڴ棬�ڴ����������ʵ���£��ǷǷ�ָ��ԭ��λ�õ��ڴ�
	int* p3 = new int(300);
	delete p3;
	if (p3) std::cout << "p3 is ָ��δ֪�ڴ棡" << std::endl;
	else std::cout << "p3 is nullptr!" << std::endl;

	// c) reset()���ָ� / ��λ / ����
	// 1) �� reset() ��������ʱ���ͷ�����ָ����ָ��Ķ��󣬲�������ָ���ÿ�
	std::unique_ptr<int> up4(new int(400));
	up4.reset();
	// 2) �� reset() ������ʱ���ͷ�����ָ����ָ��Ķ��󣬲��ø�����ָ��ָ���´���Ķ���
	std::unique_ptr<int> up5(new int(500));
	std::unique_ptr<int> up6(new int(600));
	up6.reset(up5.release());
	std::cout << "*up6 = " << *up6 << std::endl;
	up6.reset(new int(123456));
	std::cout << "*up6 = " << *up6 << std::endl;
}

void test6_2() {
	// (6.2.1) unique_ptr ���ò���(2)
	// a) = nullptr���ͷ�����ָ����ָ��Ķ��󣬲�������ָ���ÿ�
	std::unique_ptr<int> p1(new int(100));
	p1 = nullptr;

	// b) ָ��һ������
	// ʵ������������������ int��float ��ģ��������Ϳ���ʹ�� int �� int[]
	// ��������ʵ���Ͽ���ʹ�� delete ɾ���������Ͷ�̬���飬�������Ƽ�����������ʹ�� int[] ��Ϊģ������
	std::unique_ptr<int[]> p2(new int[10]);
	for (int i = 0; i < 10; ++i) {
		p2[i] = i;
		std::cout << p2[i];
	}
	std::cout << std::endl;
	std::cout << std::endl;
	// ע�⣬�˴�����ʹ�ÿ���Խ�磬������Ϊ����ָ�뱾��������±��Ƿ�Խ��
	// ʵ���������±�Խ����ʷǷ��ڴ棬�ᵼ��ϵͳ���صĳ�ʱ�俨�ٲ������쳣
	//for (int i = 0; i < 15; ++i) {
	//	p2[i] = i;
	//	std::cout << p2[i];
	//}
	//std::cout << std::endl;
	// ʵ�����������Զ���������û��ʵ������������ʹ��Ĭ�ϵ���������ʱ�������� T ������Ϊģ�����
	// ԭ��Ҳ��Ҳ�ǿ��Ե��� delete ɾ����̬���飬������Ȼ���Ƽ�����������ʹ�� T[] ��Ϊ����ָ���ģ�����
	std::unique_ptr <_nmsp1::A[]> p3(new _nmsp1::A[10]);
	// �ܽ᣺һ��ʹ�ô� [] ��������Ϊ��̬��������ָ���ģ�����
	std::cout << std::endl;
	
	// c) get()����������ָ�����������ָ��
	// d) ��ַ������ *����ȡ����ָ��ָ��Ķ���ֱ�Ӳ���
	std::unique_ptr <std::string> p4(new std::string("abc"));
	std::string* p5 = p4.get();
	// ͨ�� p5 ���� p4;
	*p5 = "Hello!";
	std::cout << "p4 = " << *p4 << std::endl;
	std::cout << std::endl;

	// e) swap()��������������ָ����ָ��Ķ���
	std::unique_ptr<std::string> up1(new std::string("123"));
	std::unique_ptr<std::string> up2(new std::string("abc"));
	std::cout << "*up1 = " << *up1 << std::endl;
	std::cout << "*up2 = " << *up2 << std::endl;
	up1.swap(up2);
	// std::swap(up1, up2);
	std::cout << " After Swap up1 and up2!" << std::endl;
	std::cout << "*ps1 = " << *up1 << std::endl;
	std::cout << "*ps2 = " << *up2 << std::endl;
	std::cout << std::endl;

	// f) ����ָ������Ϊ�ж�����
	std::unique_ptr<int> up3(new int(300));
	if (up3) std::cout << "*up3 = " << *up3 << std::endl;
	up3.reset();
	if (!up3) std::cout << "up3 is empty!" << std::endl;
	std::cout << std::endl;

	// g) unique_ptr ת���� shared_ptr ����
	// ��� unique_ptr Ϊ��ֵ���ɽ��丳ֵ�� shared_ptr��shared_ptr ��ӹ� unique_ptr ӵ�еĶ���
	// ��Ϊ unique_ptr �����ռ��������俽���� shared_ptr ���ƻ������ռ��
	// ����ֻ��ͨ�� move ���彫�����Ķ����ƽ��� shared_ptr
	// ����ֵ��������ʱֵ����ֵ���������ڽ������ƽ��� share_ptr
	// �˴��ᴴ��һ�����ƿ�
	std::shared_ptr<std::string> sp = _nmsp3::func();
	std::unique_ptr<std::string> up4(new std::string("Hello!"));
	// ʹ�� move ���彫��ֵ����תΪ��ֵ
	std::shared_ptr<std::string> sp2 = std::move(up4);
}

void test6_3() {
	// (6.3.1) �������� uniqu_ptr (���ؽ���ֵ��
	// ��ʱ�Ƿ����˿��������ɣ���
	auto up = _nmsp3::return_unique();

	// (6.3.2) ָ��ɾ�������ɵ��ö���
	// unique_ptr ��ʽ��unique_ptr<T, deleter> var(arg, deleter);
	// shared_ptr ��ʽ��shared_ptr<T> variable(arg, deleter);
	typedef void(*del1)(std::string*&);
	std::unique_ptr<std::string, del1> up1(new std::string("Hello!"), _nmsp4::deleter_function);
	using del2 = void(*)(std::string*&);
	std::unique_ptr<std::string, del2> up2(new std::string("Hello!"), _nmsp4::deleter_function);
	// ����decltype ���ص��Ǻ������ͣ����Ա���� *
	typedef decltype(_nmsp4::deleter_function)* del3;
	std::unique_ptr<std::string, del3> up3(new std::string("Hello!"), _nmsp4::deleter_function);
	std::unique_ptr<std::string, decltype(_nmsp4::deleter_function)*> up4(new std::string("Hello!"), _nmsp4::deleter_function);
	auto deleter_lambda = [](std::string*& p)->void {
		std::cout << "deleter_lambda() ִ�У�" << std::endl;
		delete p;
		p = nullptr;
	};
	std::unique_ptr<std::string, decltype(deleter_lambda)> up5(new std::string("Hello!"), deleter_lambda);
	std::unique_ptr<std::string, _nmsp4::deleter_class> up6(new std::string("Hello!"), _nmsp4::deleter_class());

	// �������� shared_ptr����ʹ������ָ����ͬ���͵�ɾ������ֻҪ������ָ������������ͬ������������ͬһ���ͣ��໥֮����Կ�������ֵ���ƽ�����Ȩ��move)
	// ������ nuique_ptr ��ͬ�� �����ڱ�����������ʱ���� nuique_ptr ��ģ������д�����ɾ���������ͣ�����ɾ��������Ҳ���� nuique_ptr ���͵�һ����
	// ����ζ�Ŷ�����ָ��Ķ���������ͬ��ɾ�������Ͳ�ͬ�� unique_ptr�� ���������ڲ�ͬ���͵ģ��໥֮�䲻���ƽ�����Ȩ(move)
	// up4, up5 ������ͬ������ move �Ϸ�
	up4 = std::move(up5);
	// up5, up6 ���Ͳ�ͬ������ move ���Ϸ�
	// up5 = std::move(up6);
	
	// (6.3.3) �ߴ�����
	int* p;
	// һ������£�unique_ptr ռ�õĿռ��С�� 4 �ֽ�
	std::unique_ptr<std::string> ps = std::make_unique<std::string>("Hello!");
	std::cout << "sizeof(p) = " << sizeof(p) << std::endl;
	std::cout << "sizeof(ps) = " << sizeof(ps) << std::endl;

	// ���������ɾ��������ռ��п������󣬶�Ч�ʻ����Ӱ��
	// �ú���ָ����Ϊɾ����ռ�ÿռ����� 8 �ֽ�
	std::cout << "function�� sizeof(up4) = " << sizeof(up4) << std::endl;
	// ���� 4 �ֽ�
	std::cout << "lambda: sizeof(up5) = " << sizeof(up5) << std::endl;
	// ���� 4 �ֽ�
	std::cout << "class: sizeof(up6) = " << sizeof(up6) << std::endl;
	
	// (6.3.4) �ܽ�
	// 1) ����ָ�����ҪĿ���ǰ����ͷ��ڴ棬��ֹ�����ͷ��ڴ浼���ڴ�й¶���������ͷſ��������ͷŵ��ڴ棨Ҳ�� RALL ��˼�룩
	// 2) auto_ptr �� C++98 ʱ��������ָ�룬���� unique_ptr ��һ�������ԣ�����������������ԭ������
	// a) �����������б���
	// b) ���ܴӺ����У���Ϊ����ֵ������
	// c) ֧�ֿ������캯����ֵ�����������ʵ�������� move ���壬����ɸ�ֵ���󲻿���ʹ��
	// �ܽ᣺auto_ptr ����������⣬�������ã�����Ǳ�ڵĳ�����������⣬���� C++11 ��������
	// �������Ҫ���ö��ָ��ͬһ�������ָ�룬Ӧ����ѡ shared_ptr;
	// ���������Ҫ���ָ��ͬһ�������ָ�룬Ӧ����ѡ unique_ptr;
}

int main() {
	// һ��ֱ��ʹ�� new / delete �����ڴ�	
	// test1();

	// ����new / delete ̽��
	// test2();
	
	// ��������ָ���� shared_ptr
	// (3.1) ����ָ������� shared_ptr ����
	// test3_1();

	// (3.2) shared_ptr ���ü��������Ӻͼ���
	// test3_2();

	// (3.3) shared_ptr ���ò���
	// test3_3();

	// (3.4) ָ��ɾ�����Լ� share_ptr ����̬���������
	// test3_4();

	// �ġ�weak_ptr
	// (4.1) weak_ptr ����
	// test4_1();

	// (4.2) weak_ptr ���ò�����ߴ�����
	// test4_2();

	// �塢shared_ptr �������
	// (5.1) ������ָ���� get() ����
	// test5_1();
	
	// (5.2) ����ָ�� this �� ѭ������
	// test5_2();

	// (5.3) ����˵���� move ����
	// test5_3();

	// ����unique_ptr 
	// (6.1) unique_ptr �����볣�ò���(1)
	// test6_1();

	// (6.2) unique_ptr ���ò���(2)
	// test6_2();

	// (6.3) 
	test6_3();

	return 0;
}

