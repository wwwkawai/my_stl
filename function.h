//
// Created by 87549 on 2023/1/16.
//

#ifndef MY_STL_FUNCTION_H
#define MY_STL_FUNCTION_H
namespace my_stl {
    template<class Arg, class Result>
    struct unary_function {
        typedef Arg argument_type;
        typedef Result result_type;
    };
    template<class Arg1, class Arg2, class Result>
    struct binary_function {
        typedef Arg1 first_argument_type;
        typedef Arg2 second_argument_type;
        typedef Result result_type;
    };


    template<class T>
    struct plus : public binary_function<T, T, T> {
        T operator()(const T &x, const T &y) const {
            return x + y;
        }
    };

    // minus, multiplies......
    //证同元素
    template<class T>
    inline T identity_element(plus<T>) {
        return T(0);
    }

    template<class T>
    struct equal_to : public binary_function<T, T, bool> {
        bool operator() (const T& x, const T& y){
            return x==y;
        }
    };
    // not_equal_to, less......

    template<class Arg1, class Arg2>
    struct project1st:public binary_function<Arg1, Arg2, Arg1>{
        Arg1 operator() (const Arg1& x, const Arg2&){
            return x;
        }
    };

}



#endif //MY_STL_FUNCTION_H
