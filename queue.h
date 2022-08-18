
#ifndef MY_STL_QUEUE_H
#define MY_STL_QUEUE_H
#include "deque.h"
namespace my_stl{
    template <class T, class Sequence = deque<T> >
    class queue {
        friend bool operator == <> (const queue& x, const queue& y){
            return x.c == y.c;
        }
        friend bool operator < <> (const queue& x, const queue& y){
            return x.c < y.c;
        }

    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        typedef typename Sequence::reference reference;
    protected:
        Sequence c;
    public:
        bool empty() const {
            return c.empty();
        }
        size_type size() const {
            return c.size();
        }
        reference front() const {
            return c.front();
        }
        reference back() const {
            return c.back();
        }
        void push(const value_type& x) {
            c.push_back(x);
        }
        void pop() {
            c.pop_front();
        }
    };
}
#endif //MY_STL_QUEUE_H
