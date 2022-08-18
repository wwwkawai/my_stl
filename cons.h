#ifndef MY_STL_CONS_H
#define MY_STL_CONS_H
#include <new.h>
#include "type_traits.h"
namespace my_stl {
    template<class T1, class T2>
    //placement new;调用T1::T1(value)
    inline void construct(T1 *p, const T2 &value) {
        new(p) T1(value);
    }

    template<class T>
    //第一版，接受一个指针
    inline void destroy(T *p) {
        p->~T();
    }

    template<class ForwardIterator>
    //第二版，接受前后两个迭代器，试图找出元素类型，进而利用_type_traits<>求取最适当方式
    inline void destroy(ForwardIterator begin, ForwardIterator end) {
        _destroy(begin, end, value_type(begin));
    }

    template<class ForwardIterator, class T>
    //判断是否有trivial destructor
    inline void _destroy(ForwardIterator begin, ForwardIterator end, T *) {
        typedef typename _type_traits<T>::has_trivial_destructor trivial_destructor;//typename表面后面是一个类型名称
        _destroy_aux(begin, end, trivial_destructor());
    }

    template<class ForwardIterator>
    //如果有non_trivial destructor
    inline void _destroy_aux(ForwardIterator begin, ForwardIterator end, _false_type) {
        for (; begin < end; ++begin)
            destroy(&*begin);
    }

    template<class ForwardIterator>
    //如果有trivial destructor,什么也不做
    inline void _destroy_aux(ForwardIterator begin, ForwardIterator end, _true_type) {}

// 第二版destory 泛型特化
    inline void destroy(char *, char *) {}

    inline void destroy(wchar_t *, wchar_t *) {}

    inline void destroy(int *, int *) {}

    inline void destroy(long *, long *) {}

    inline void destroy(float *, float *) {}

    inline void destroy(double *, double *) {}
}
#endif //MY_STL_CONS_H
