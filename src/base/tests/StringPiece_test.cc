#include <vector>
#include <iostream>
#include <algorithm>

#include "Util.h"
#include "StringPiece.h"

using namespace mrpc;

void TestStringArg()
{
    Util::PrintTitle("Test StringArg");
    const char* str1 = "const char*";
    char str2[] = "char[]";
    std::string str3 = "std::string";
    StringArg s1 = str1;
    StringArg s2 = str2;
    StringArg s3 = str3;
    std::cout << s1.c_str() << std::endl;
    std::cout << s2.c_str() << std::endl;
    std::cout << s3.c_str() << std::endl;

    StringArg s4(s1);
    StringArg s5(s1); s5 = s2;
    StringArg s6 = std::move(s3);
    std::cout << s4.c_str() << std::endl;
    std::cout << s5.c_str() << std::endl;
    std::cout << s6.c_str() << std::endl;
}

void TestStringPiece()
{
    Util::PrintTitle("Test StringPiece");
    const int stringNum = 10;
    std::vector<std::string> strings;
    std::vector<StringPiece> stringPieces;
    for(int i = 0; i < stringNum; ++i)
    {
        strings.push_back(Util::RandomString(10));
        stringPieces.push_back(StringPiece(strings[i]));
    }
    std::cout << "Sort stringPieces!" << std::endl;
    std::sort(stringPieces.begin(), stringPieces.end());
    for(int i = 0; i < stringNum; ++i)
    {
        std::cout << "[" << strings[i] << "; " 
                  << stringPieces[i].data() << "]\n";
    }
    std::cout << std::endl;

    std::cout << "Sort strings!" << std::endl;
    std::sort(strings.begin(), strings.end());
    for(int i = 0; i < stringNum; ++i)
    {
        std::cout << "[" << strings[i] << "; " 
                  << stringPieces[i].data() << "]\n";
    }
}

int main()
{
    TestStringArg();
    TestStringPiece();
    return 0;
}