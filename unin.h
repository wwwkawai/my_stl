#ifndef MY_STL_UNIN_H
#define MY_STL_UNIN_H
//以下函数将内存的配置与对象的构造行为分离开
//要么产生所有必要的元素，要么不产生任何元素
//此处省略了异常处理
#include "cons.h"
namespace my_stl {
    template<class InputIterator, class ForwardIterator>
    inline ForwardIterator
    _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _true_type) {
        return copy(first, last, result); //调用stl的copy();
    }

    template<class InputIterator, class ForwardIterator>
    inline ForwardIterator
    _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _false_type) {
        ForwardIterator cur = result;
        for (; first != last; ++first, ++cur)
            construct(&*cur, *first); //一个一个元素构造
        return cur;
    }

    template<class InputIterator, class ForwardIterator, class T>
    inline ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T *) {
        typedef typename _type_traits<T>::is_POD_type is_POD;
        return _uninitialized_copy_aux(first, last, result, is_POD());
    }

    template<class InputIterator, class ForwardIterator>
    ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result) {
        return _uninitialized_copy(first, last, result, value_type(result));
    }


    template<class ForwardIterator, class T>
    void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T &x, _true_type) {
        fill(first, last, x);
    }

    template<class ForwardIterator, class T>
    void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T &x, _false_type) {
        ForwardIterator cur = first;
        for (; cur != last; ++cur)
            construct(&*cur, x);
    }

    template<class ForwardIterator, class T, class T1>
    void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &x, T1 *) {
        typedef typename _type_traits<T1>::is_POD_type is_POD;
        _uninitialized_fill_aux(first, last, x, is_POD());
    }

    template<class ForwardIterator, class T>
    void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &x) {
        _uninitialized_fill(first, last, x, value_type(first));
    }


//POD
    template<class ForwardIterator, class Size, class T>
    inline ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n, const T &x, _true_type) {
        return fill_n(first, n, x); //交由高阶函数执行
    }

//non_POD
    template<class ForwardIterator, class Size, class T>
    inline ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n, const T &x, _false_type) {
        ForwardIterator cur = first;
        for (; n > 0; --n, ++cur)
            construct(&*cur, x);
        return cur;
    }


    template<class ForwardIterator, class Size, class T, class T1>
    inline ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T &x, T1 *) {
        typedef typename _type_traits<T1>::is_POD_type is_POD;
        return _uninitialized_fill_n_aux(first, n, x, is_POD());
    }

    template<class ForwardIterator, class Size, class T>
    ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T &x) {
        return _uninitialized_fill_n(first, n, x, value_type(first));
        //value_type()判断是否是POD（plain old data）型别，POD型别必有trivial ctor/dtor/copy/assignment函数，可直接采取填写初值手法
    }
}
#endif //MY_STL_UNIN_H
