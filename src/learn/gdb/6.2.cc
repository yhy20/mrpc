#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>

using namespace std;

/// print STL 中的容器
int main()
{
    vector<int> vec(5);
    map<int, int> mp = { {1, 2}, {3, 4}, {5, 6} };
    set<int> set;
    queue<int> q;
    stack<int> s;
    for(int i = 0; i < vec.size(); ++i)
    {
        vec[i] = i;
        set.insert(i);
        q.push(i);
        s.push(i);
    }

    cout << "vec contains:";
    for(int i = 0; i < vec.size(); ++i)
    {
        cout << ' ' << vec[i]; 
    }
    cout << '\n';

    return 0;
}