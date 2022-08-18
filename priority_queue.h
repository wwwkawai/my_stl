#ifndef MY_STL_PRIORITY_QUEUE_H
#define MY_STL_PRIORITY_QUEUE_H
#include "vector.h"
#include "heap.h"
//暂不支持改变compare
namespace my_stl{
    template <class T, class Sequence = vector<T>>
    class priority_queue {
    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        typedef typename Sequence::reference reference;
    protected:
        Sequence c;
    public:
        priority_queue() : c() {}
        priority_queue(InputIterator first, InputIterator last) : c(first, last) {
            make_heap(c.begin(), c.end();
        }
        bool empty() const {
            return c.empty();
        }
        size_type size() const {
            return c.size();
        }
        reference top() {
            return c.front();
        }
        void push(const value_type& x) {
            c.push_back(x);
            push_heap(c.begin(),c.end());
        }
        void pop() {
            pop_heap(c.begin(), c.end());
            c.pop_back();
        }
    };
}
#endif //MY_STL_PRIORITY_QUEUE_H
