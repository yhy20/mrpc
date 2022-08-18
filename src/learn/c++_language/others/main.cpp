#include <iostream>
#include <map>
#include <functional>

namespace _nmsp1 {
	class BiggerThanZero {
	public:
		int operator()(int tv) {
			return tv >= 0 ? tv : 0;
		}
		
		// ���� operator() �����
		// ����Ҫ���ڲ����������������������𼴿�
		// ע�⣬���ص����Ͳ�ͬ�������������غ���
		int operator()(int tv1, int tv2) {
			return tv1 + tv2;
		}

		int LessThanZero(int tv) {
			return tv <= 0 ? tv : 0;
		}
	};

	// operator() �����ǳ�Ա����
	// void operator()(BiggerThanZero b){}
	
	// class LessThanZero {
	// public:
	//	 // operator() ����������Ǿ�̬��Ա����
	//	 static int operator()(int tv) {
	//		 return tv <= 0 ? tv : 0;
	//	 }
	// };

	// echo_value ���β����������͡�����ֵ�� BiggerThanZero ���ص� operator() ��ȫ��ͬ
	// Ҳ��Ϊ������ʽ��ͬ int(int)
	int echo_value(int tv) {
		// std::cout << "tv = " << tv << ::std::endl;
		return tv;
	}

	int overload(int tv1) {
		return tv1 * 2;
	}

	int overload() {
		return 0;
	}
}

void test1() {
	// (1.1) operator() �������������
	// ��������������� operator() ��������������ʹ�ú���һ��ʹ�������Ķ���
	// ע�� operator() �����ǳ�Ա������ operator() �����Ǿ�̬��Ա����
	// ������ operator() ������Ķ���Ҳ�к������󣨿ɵ��ö���
	_nmsp1::BiggerThanZero obj;
	// ��ʽ���� operator()
	std::cout << obj(5) << std::endl;
	// ��ʽ���� operator()
	std::cout << obj.operator()(5) << std::endl;
	// ������ʱ������ʽ���õ��� operator()
	std::cout << _nmsp1::BiggerThanZero()(5) << std::endl;
	// ������ʱ������ʽ���õ��� operator()
	std::cout << _nmsp1::BiggerThanZero().operator()(5)<< std::endl;
	std::cout << std::endl;
	// (1.2) ��ͬ���ö���ĵ�����ʽ��ͬ
	// ���� echo_value �� BiggerThanZero ���ص� operator() ������ʽ��ȫ��ͬ
	// һ�ֵ�����ʽ��Ӧһ�ֺ������ͣ����������ɷ���ֵ���βι�ͬ����
	// ���� echo_value �� BiggerThanZero ���ص� operator() �ĺ������Ͷ���  int(int)
	
	// �ɵ��ö��������ͨ����������ָ�롢lambda ���ʽ��bind ���󡢺�������
	// ��ͳһ�ķ�ʽ��������ͬ������ʽ�ɵ��ö��󱣴�����
	// ����һ������ָ��
	int(*p)(int) = _nmsp1::echo_value;
	// ����һ������
	typedef int(*func)(int);
	func p1 = _nmsp1::echo_value;

	// ������Ҫ������������ֿɵ��ö���
	std::map<std::string, func> mp;
	mp.insert({ "1) int(int)", p });
	mp.insert({ "2) int(int)", _nmsp1::echo_value});
	// ����һ�����⣬��ΰ�һ���������󱣴����������Ȼ����Ĵ��벻����
	// mp.insert({"3) int(int)", _nmsp1::BiggerThanZero})
	// ����һ������ΰ�һ����Ա�������������

	// (1.3) function ��ģ��
	// ���ݸ� function ��ģ�������ʾ function �ܹ���Ŀɵ��ö���ĵ�����ʽ
	// a) ����ָ��
	std::function<int(int)> f1 = p;
	// b) ��ͨ����
	std::function<int(int)> f2 = _nmsp1::echo_value;
	// c) ��������
	std::function<int(int)> f3 = _nmsp1::BiggerThanZero();
	// d) lambda
	std::function<int(int)> f4 = [](int tv)->int { return tv * 2; };
	// e) bind ����
	std::function<int(int)> f5 = std::bind(&_nmsp1::BiggerThanZero::LessThanZero, &obj, std::placeholders::_1);

	std::map<std::string, std::function<int(int)>> mp1 = {
		{"f1", p},
		{"f2", _nmsp1::echo_value},
		{"f3", _nmsp1::BiggerThanZero()},
		{"f4", [](int tv)->int { return tv * 2; }},
		{"f5", std::bind(&_nmsp1::BiggerThanZero::LessThanZero, &obj, std::placeholders::_1)}
	};
	std::cout << "f1(10) = " << mp1["f1"](10) << std::endl;
	std::cout << "f2(10) = " << mp1["f2"](10) << std::endl;
	std::cout << "f3(10) = " << mp1["f3"](10) << std::endl;
	std::cout << "f4(10) = " << mp1["f4"](10) << std::endl;
	std::cout << "f5(10) = " << mp1["f5"](10) << std::endl;

	// function ������ͨ��������ʱ���޷�ʶ������һ������
	// Ҳ����˵ģ������������ֺ����βΣ��ж����ԣ�
	// std::function<int(int)> fv = _nmsp1::overload;

	// ���������ʹ�ú���ָ��
	int(*p2)(int) = _nmsp1::overload;
	int(*p3)() = _nmsp1::overload;
	// ͨ�����ݺ���ָ���ӵĴ��� overload() ��������ȷ�汾
	std::function<int(int)> fv = p2;
}

void test2() {

}

void test3() {

}

void test4() {
	// (4.1) ������ȡ������type traits) (C++11)
	// ������ȡ������ STL Դ��ʵ����Ӧ�ý϶࣬��Ҫ������ȡһЩ��Ϣ
}

int main() {
	// һ��function
	// test1();

	// �����ɵ��ö�����bind
	// test2();

	// ����
	// test3();

	// �ġ�traits
	test4();
	return 0;
}