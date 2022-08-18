#include <iostream>

template <typename T>
class B;

class A 
{   
    friend class B<double>;
private:
    void print()
    {
        std::cout << "A::print()!" << std::endl;
    }
};

template <typename T>
class B
{
public:

};

template <>
class B<double>
{
public:
    void useA()
    {
        A a;
        a.print();
    }
};

int main()
{   
    B<double> b;
    b.useA();
    return 0;
}