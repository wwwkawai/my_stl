//
// Created by 87549 on 2023/1/16.
//

#ifndef MY_STL_ADAPTER_H
#define MY_STL_ADAPTER_H
#include "function.h"

namespace my_stl {
    template<class Predicate>
    class unary_negate : public unary_function<typename Predicate::argument_type, bool> {
    protected:
        Predicate pred;
    public:
        explicit unary_negate(const Predicate &x) : pred(x) {}

        bool operator()(const typename Predicate::argument_type &x) const {
            return !pred(x);
        }
    };

    template<class Predicate>
    inline unary_negate<Predicate> not1(const Predicate &pred) {
        return unary_negate<Predicate>(pred);
    }

    //not2 同理

    template<class Operation>
    class binder1st : public unary_function<typename Operation::second_argument_type, typename Operation::result_type> {
    protected:
        Operation op;
        typename Operation::first_argument_type value;
    public:
        binder1st(const Operation &x, const typename Operation::first_argument_type &y) : op(x), value(y) {}

        typename Operation::result_type operator()(const typename Operation::second_argument_type &x) const {
            return op(value, x);
        }
    };

    template<class Operation, class T>
    inline binder1st<Operation> bind1st(const Operation &op, const T &x) {
        typedef typename Operation::first_argument_type arg1_type;
        return binder1st<Operation>(op, arg1_type(x));
    }

    template<class Operation1, class Operation2>
    class unary_compose : public unary_function<typename Operation2::argment_type, typename Operation1::result_type> {
    protected:
        Operation1 op1;
        Operation2 op2;
    public:
        unary_compose(const Operation1 &x, const Operation2 &y) : op1(x), op2(y) {}

        typename Operation1::result_type operator()(const typename Operation2::argument_type &x) const {
            return op1(op2(x));
        }

    };

    template<class Operation1, class Operation2>
    inline unary_compose<Operation1, Operation2> compose1(const Operation1 &op1, const Operation2 &op2) {
        return unary_compose<Operation1, Operation2>(op1, op2);
    }


    template<class Arg, class Result>
    class pointer_to_unary_function : public unary_function<Arg, Result> {
    protected:
        Result (*ptr)(Arg);

    public:
        pointer_to_unary_function() {}

        explicit pointer_to_unary_function(Result(*x)(Arg)) : ptr(x) {}

        Result operator()(Arg x) const {
            return ptr(x);
        }
    };

    template<class Arg, class Result>
    inline pointer_to_unary_function<Arg, Result> ptr_fun(Result(*x)(Arg)) {
        return pointer_to_unary_function<Arg, Result>(x);
    }

    template<class S, class T>
    class mem_fun_t : public unary_function<T *, S> {
    public:
        explicit mem_fun_t(S(T::*pf)()) : f(pf) {}

        S operator()(T *p) const {
            return (p->*f)();
        }

    private:
        S (T::*f)();
    };

    template<class S, class T>
    inline mem_fun_t<S, T> mem_fun(S(T::*f)()) {
        return mem_fun_t < S, T > (f);
    }
}
#endif //MY_STL_ADAPTER_H
