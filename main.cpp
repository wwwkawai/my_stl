#include <iostream>
#include "vector.h"
using namespace  my_stl;
int main() {
    vector<int> a;
    a.push_back(1);
    a.push_back(2);
    std::cout<<a.size()<<std::endl;
    a.pop_back();
    std::cout<<a.back()<<std::endl;
    std::cout<<a.size()<<std::endl;
    a.erase(a.begin());
    std::cout<<a.size()<<std::endl;
    return 0;
}