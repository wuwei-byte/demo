#include<iostream>
#include "KlruCache.h"
#include "KICachePolicy.h"

int main()
{
    auto obj=new LruNode(10,20);
    std::cout<<obj->getValue()<<"\n";
    std::cout<<"Hello World\n"<<std::endl;
    return 0;
}