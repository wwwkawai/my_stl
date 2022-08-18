#ifndef MY_STL_VECTOR_H
#define MY_STL_VECTOR_H
#include "alloc.h"
#include "iterator.h"
#include "cons.h"
#include "unin.h"
#include <cstddef>
namespace my_stl{
    template <class T, class Alloc = alloc>
    class vector {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef value_type* iterator;
        typedef value_type& reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
    protected:
        //my_alloc作为空间配置器
        typedef my_alloc<value_type, Alloc> data_allocator;
        iterator start; //使用空间的头
        iterator finish; //使用空间的尾
        iterator end_of_storage; //可用空间的尾

        void insert_aux(iterator position, const T& x);
        void deallocate() {
            if(start)
                data_allocator::deallocate(start, end_of_storage - start);
        }
        void fill_initialize(size_type n, const T& value) {
            start = allocate_and_fill(n, value);
            finish = start + n;
            end_of_storage = finish;
        }

    public:
        iterator begin() { return start; }
        iterator end() { return finish; }
        size_type size()  { return size_type (end() - begin()); }
        size_type capacity() const { return size_type (end_of_storage - begin()); }
        bool empty() { return begin()==end(); }
        reference operator[] (size_type n) { return *(begin() + n); }
        vector() : start(0), finish(0), end_of_storage(0) {}
        vector(size_type n, const T& value) { fill_initialize(n, value); }
        vector(int n, const T& value) { fill_initialize(n, value); }
        vector(long n, const T& value) { fill_initialize(n,value); }
        explicit vector(size_type n) { fill_initialize(n,T()); }
        ~vector() {
            destroy(start, finish);
            deallocate();
        }
        reference front() { return *begin(); }  //第一个元素
        reference back() { return *(end() - 1); } //最后一个元素
        void push_back(const T& x) {
            if(finish != end_of_storage) {
                construct(finish, x);
                ++finish;
            } else{
                insert_aux(end(), x);
            }
        }
        void pop_back() {
            --finish;
            destroy(finish);
        }
        //区间删除
        iterator erase(iterator first, iterator last) {
            iterator i = copy(last, finish, first);
            destroy(i, finish);
            finish = finish - (last - first);
            return first;
        }
        //单点删除
        iterator erase(iterator position) {
            if(position + 1 != end())
                copy(position + 1, finish, position);
            --finish;
            destroy(finish);
            return position;
        }
        void resize(size_type new_size, const T& x) {
            if(new_size < size())
                erase(begin() + new_size, end());
            else
                insert(end(), new_size - size(), x);
        }
        //从position开始插入n个元素
        void insert(iterator position, size_type n ,const T& x) {
            if(n != 0) {
                if(size_type(end_of_storage - finish) >= n) {
                    //备用空间足够
                    T x_copy = x;
                    const size_type elems_after = finish - position; //插入点后的元素个数
                    iterator old_finish = finish;
                    if(elems_after > n) {
                        //如果插入点后的元素个数大于n
                        uninitialized_copy(finish - n ,finish, finish);
                        finish += n; //finish 后移
                        copy_backward(position, old_finish - n, old_finish);
                        fill(position, position + n, x_copy); //插入新值
                    }else{
                        //插入点后的元素个数小于等于n
                        uninitialized_fill_n(finish, n - elems_after, x_copy);
                        finish += n - elems_after;
                        uninitialized_copy(position, old_finish, finish);
                        finish += elems_after;
                        fill(position, old_finish, x_copy);
                    }
                }else{
                    //备用空间不足
                    //决定新空间的长度
                    const size_type old_size = size();
                    const size_type len = old_size + max(old_size, n);
                    //配置新的空间
                    iterator new_start = data_allocator::allocate(len);
                    iterator new_finish = new_start;
                    try{
                        //copy前面的元素->fill插入元素->copy后面的元素
                        new_finish = uninitialized_copy(start, position, new_start);
                        new_finish = uninitialized_fill_n(new_finish, n ,x);
                        new_finish = uninitialized_copy(position, finish, new_finish);

                    }
                    catch(...) {
                        destroy(new_start, new_finish);
                        data_allocator::deallocate(new_start, len);
                        throw;
                    }
                    //清除并释放旧的vector
                    destroy(start, finish);
                    start = new_start;
                    finish = new_finish;
                    end_of_storage = new_start + len;
                }
            }
        }
        void insert(iterator position, const T& x) {
            insert(position, 1, x);
        }
        void resize(size_type new_size) { resize(new_size, T()); }
        void clear() { erase(begin(), end()); }

    protected:
        //配置空间并填满内容
        iterator allocate_and_fill(size_type n, const T& x) {
            iterator result = data_allocator::allocate(n);
            uninitialized_fill_n(result, n, x);
            return result;
        }

    };
    template <class T, class Alloc>
    void vector<T, Alloc>::insert_aux(iterator position, const T& x) {
        if(finish != end_of_storage) {
            construct(finish, *(finish - 1));
            ++finish;
            T x_copy = x;
            copy_backward(position, finish - 2, finish - 1);
            *position = x_copy;
        }else{
            //若原大小不为0，则变为原来的两倍，否则变为1
            const size_type old_size = size();
            const size_type len = old_size != 0 ? 2 * old_size : 1;
            iterator new_start = data_allocator::allocate(len) ;
            iterator new_finish = new_start;
            try {
                //将原来的内容copy到新的位置
                new_finish = uninitialized_copy(start, position, new_start);
                construct(new_finish, x);
                ++new_finish;
                new_finish = uninitialized_copy(position, finish, new_finish);
            }
            catch (...) {
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }
            //析构并释放原vector
            destroy(begin(), end());
            deallocate();
            //调整迭代器，指向新vector
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + len;
        }
    }
}
#endif //MY_STL_VECTOR_H
