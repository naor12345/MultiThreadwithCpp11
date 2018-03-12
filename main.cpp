#include "threadpool.h"
#include <iostream>
#include <limits.h>

using namespace std;

int f(int m)
{
    for(int i = 0; i<INT_MAX; i++){}
    return m;
}

int main()
{
    threadpool executor{4};
    vector<future<int>> fut;
    
    for(int i = 0; i<4; i++)
    {
        fut.push_back(executor.commit(bind(f, i)));
    }
    executor.drainTask();
    
    for(auto &m : fut)
    {
        cout<<m.get()<<endl;
    }
    
    return 0;
}


