#ifndef __MY_TIME__
#define __MY_TIME__

// ������ .h �ļ��ж���ȫ�ֱ���������
// 1) ���ۣ��﷨����������У�������ܵ����ظ����������
// a) ��ͷ�ļ�����һ��Դ�ļ�ʹ�õ��������������ɿ�ִ���ļ�
// b) ��ͷ�ļ������Դ�ļ�������������ִ���� cpp��cc1��as����������(ld)ʱ��ᱨ���ظ��������⣩

// 2) ԭ������� 
// ����Ҫ��ȷ .h �ļ����ṩ��Ԥ�������ģ���Ԥ������������ɺ�Ͳ���ʹ����
// ���� .h �ļ��ı�����������Ԥ����׶α�Ƕ�뵽ÿ�� .cpp ���ṩ��������������������������
// ����������а����˱��������������ȫ���ⲿ��Ϣ����Щ��Ϣ���԰�������������﷨�ϵĴ���
// ���� .h �ļ����ܱ���� .cpp �ļ� #include����͵�����ÿ�� .cpp �ļ��ж��ᴴ�����ȫ�ֱ���
// �ڱ����Ԥ��������ͻ��׶ζ��� .cpp �ļ�����һ���������ļ��������Բ������������
// �������������ӣ�ld���׶Σ��Ὣ���е�Ŀ���ļ������������������ջᷢ��ȫ�ֱ�����������
// ���¶����Դ�������붨����صĴ���Ķ���Ӧ�÷��� .h �ļ��У�.h�ļ���ֻ�ܷ�����
//
// 3) ��ȷ������
// a) ����ȫ�ֱ�������ȷ���������� .h �ļ���Ӧ�� .cpp �ļ��зű����Ķ��壬���� .h �ļ�ʹ�� extern 
// ��佫�������ⲿ������extern ����ʾ��������Ѿ���ĳ�� .cpp �ļ��ж����ˣ��ڵ�ǰ .cpp �ļ���ֻ��Ҫʹ�ü���
// b) ���ں���Ҳ��ͬ�������Ķ������ .h �ļ���Ӧ�� .cpp �ļ��У��� .h �ļ���ʹ�� extern ���ֻ������
// c) ���������ͱȽ����⣬��ͨ��Ա������̬��Ա���������� .h �ļ�������ڲ�ʵ�֣������ڶ�Ӧ�� .cpp �ļ���ʵ��
// ������ڲ�ʵ�ֻ���ʾ������������Ϊ����(inlines)��������������� .h �ļ���ʵ�ֺ���������ڶ���������
// ������ľ�̬��Ա������������Ҫ�����ķ���ռ䣬����ֻ�ܶ��������⣬���ֻ�ܶ����� .h �ļ���Ӧ�� .cpp �ļ���

// ������ .h �ļ��ж��徲̬ȫ�ֱ���������
// 1) ���ۣ����� .h �ļ��ж��� static ȫ�ֱ�����������ִ��󣬵���̬��������������� "��ǰԴ�ļ�"��Ҳ����˵
// ��ͬ .cpp �ļ��ж����ͬ�� static ȫ�ֱ�����ʵ���Ƕ�����ÿ��Դ�ļ��Ĳ�ͬ��������������Ϊʵ�ʺ����
// 2�� ������Ҫ֪�� static ���ε�ȫ�ֺ�����ȫ�־�̬�����������޶��ڱ�Դ���ļ��У����ܱ���Դ���ļ�����Ĵ����ļ�����
// Ҳ����˵��Ҳ�Ƕ�����ÿ��Դ�ļ�����ͬԴ�ļ��е� static ����ʵ������ȫ��ͬ�ĺ������ʲ��ᵼ�´���
         
// �� .h �ļ��ж���һ��ȫ�ֱ���
// ����ж�� .cpp �ļ� #include .h �ļ����ᵼ�¶����Դ����Ѿ�֤ʵ��
// int g_count = 0;

// �� .h �ļ��ж���һ�� static ȫ�ֱ���
// ������ִ����Ѿ�֤ʵ��
// static int s_count = 0;

// �� .h �ļ��ж���һ��ȫ�ֺ���
// ����ж�� .cpp �ļ� #include .  h �ļ����ᵼ�¶����Դ����Ѿ�֤ʵ��
// int global_function(int tv) {
//	 return tv * tv;
// } 

// �� .h �ļ��ж���һ����̬ȫ�ֺ���
// ������ִ����Ѿ�֤ʵ��
// static int static_function(int tv) {
//	 return tv * tv;
// }

namespace _nmsp {
	// ��ʹ��ĳ�������ռ��£��� .h �ļ��ж���ȫ�ֱ�������Ȼ�ᵼ�¶���������
	// int g_count = 0;
	
	// ��ʹ��ĳ�������ռ��£��� .h �ļ��ж���ȫ�ֺ�������Ȼ�ᵼ�¶���������
	// int global_function(int tv) {
	//	   return tv * tv;
	// }

	// ��Ĭ�������ռ�����������ռ��¶�����ͬ�ĺ������ᵼ���ظ���������
	// static int static_function(int tv) {
	// 	 return tv * tv;
	// }

	// ǰ�����ã�����������������
	class Time;
	void deal_time(Time);

	// �ඨ��� .h �ļ������ #include ʱ�������Ϊ������
	// �ඨ�壬����������������
	class Time {
	private:
		int microsecond;

	private:
		int caculate_microsecond_from(int);

	public:
		int hour;
		int minute;
		int second;
		mutable bool valid;
		
		// ����һ����̬��Ա��������δ�����ڴ棬Ҳû�г�ʼ����
		// �������Ϊһ��ӵ����������;�̬�������ڵı���
		static int num;

	public:
		Time();
		explicit Time(int);
		explicit Time(int, int, int s = 0);
		Time(const Time&);
		Time& operator=(const Time&);
		Time& operator++();
		Time&& operator++(int);
		void init_time(int h = 0, int m = 0, int s = 0, int ms = 0, bool v = false);
		void set_timer(int);
		const int& get_hour() const;
		const int* get_minute() const;
		int get_microsecond() const;
		void output_time() const;
		void output_micro() const;

	public:
		void add_hour(int h) {
			this->hour += h;
		}
		Time* get_self() {
			return this;
		}
		/* Time* get_self(); */

	public:
		// ��̬��Ա�����ϲ�����ʹ�������޶���
		static int get_num() /* const */{
			return num;
		}
		/* static int get_num(); */
	};

	// ������ .h �ļ���ʵ�־�̬��Ա���������ڶ����Դ���
	// int Time::get_num() {
	//	 return num;
	// }
	// 
	// ������ .h �ļ���ʵ����ͨ��Ա���������ڶ����Դ���
	// Time* Time::get_self() {
	//	 return this;
	// }

	// ������ .h �ļ��ж��徲̬��Ա���������ڶ����Դ���
	// int Time::num = 0;

	template <typename T>
	const T& operator<<(T& out, const Time& time) {
		time.output_time();
		return out;
	}
	
	// Ĭ���� exyern ��
	void print(const Time&);
	extern Time call_time(Time);
}

#endif
