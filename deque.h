#ifndef MY_STL_DEQUE_H
#define MY_STL_DEQUE_H
#include "iterator.h"
#include "cons.h"
#include "alloc.h"
#include "unin.h"
//deque由分段连续的空间组成，需要分段控制维护其逻辑连续
namespace my_stl{
    //n不为0则返回n，表示buffer_size由用户定义
    //n为0则使用默认值
    //如果sz小于512，传回512/sz，否则传回1
    inline size_t _deque_buf_size(size_t n, size_t sz) {
        return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
    }
    template <class T, class Ref, class Ptr, size_t BufSiz>
    struct _deque_iterator {
        typedef _deque_iterator<T, T&, T*, BufSiz> iterator;
        typedef _deque_iterator<T, const T&, const T*, BufSiz> const_iterator;
        static size_t buffer_size() {
            return _deque_buf_size(BufSiz, sizeof(T));
        }
        typedef random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef Ptr pointer;
        typedef Ref reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef T** map_pointer;
        typedef _deque_iterator self;

        T* cur;  //当前所指元素
        T* first;  //所指缓冲区的头
        T* last;   //所指缓冲区的尾
        map_pointer node;  //指向管控中心
        //跳一个缓冲区
        void set_node(map_pointer new_node) {
            node = new_node;
            first = *new_node;
            last = first + difference_type(buffer_size());
        }
        //重载运算子
        reference operator* () const { return *cur; }
        pointer operator-> () const { return &(operator*())}
        difference_type operator- (const self& x) const {
            return difference_type(buffer_size()) * (node - x.node - 1) + (cur - first) + (x.last - x.cur);
        }
        self& operator++ () {
            ++cur;
            if(cur == last) {
                set_node(node + 1);
                cur = first;
            }
            return *this;
        }
        self operator++ (int) {
            self tmp = *this;
            ++*this;
            return tmp;
        }
        self& operator-- () {
            if(cur == first) {
                set_node(node - 1);
                cur = last;
            }
            --cur;
            return *this;
        }
        self operator-- (int) {
            self tmp = *this;
            --*this;
            return tmp;
        }
        //实现随机存取，迭代器可以跳n步
        self& operator+= (difference_type n) {
            difference_type offset = n + (cur - first);
            if(offset >= 0 && offset < difference_type(buffer_size()))
                cur += n;
            else{
                difference_type node_offset = offset > 0 ? offset / difference_type (buffer_size()) : -difference_type((-offset - 1) / buffer_size()) - 1;
                set_node(node_offset + node);
                cur = first + (offset - node_offset * difference_type(buffer_size()));
            }
            return *this;
        }
        self operator+ (difference_type n) const {
            self tmp = *this;
            return tmp += n;
        }
        self& operator-= (difference_type n) {
            return *this += -n;
        }
        self operator- (difference_type n) {
            self tmp = *this;
            return tmp -= n;
        }
        reference operator[] (difference_type n) const {
            return *(*this + n);
        }
        bool operator== (const self& x) const {
            return cur == x.cur;
        }
        bool operator!= (const self& x) cons {
            return !(*this == x);
        }
        bool operator< (const self& x) const {
            return (node == x.node) ? (cur < x.cur) : (node < x.node);
        }
    };
    template <class T, class Alloc = alloc, size_t BufSiz = 0>
    class deque {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef ptrdiff_t difference_type;
        typedef size_t size_type;
        typedef _deque_iterator<T, T&, T*, BufSiz> iterator;
    protected:
        typedef pointer* map_pointer;
    protected:
        iterator start;  //第一个节点
        iterator finish;  //最后一个节点
        map_pointer map;  //指向map，map是连续空间，其中每个元素指向一个缓冲区
        size_type map_size; //map内指针数量
    protected:
        //专属配置器
        typedef my_alloc<value_type, Alloc> data_allocator; //一个元素大小
        typedef my_alloc<pointer, Alloc> map_allocator;  //一个指针大小
    protected:
        static size_t buffer_size() {
            return _deque_buf_size(BufSiz, sizeof(T));
        }
        T* allocate_node() {
            return data_allocator::allocate(buffer_size());
        }
        void deallocate_node(T* p) {
            data_allocator::deallocate(p, buffer_size());
        }
        void push_back_aux(const value_type& t);
        void push_front_aux(const value_type& t);
        void pop_back_aux();
        void pop_front_aux();
        iterator insert_aux(iterator pos, const value_type& x);
        void reserve_map_at_back(size_type nodes_to_add = 1) {
            if(nodes_to_add + 1 > map_size - (finish.node - map))
                reallocate_map(nodes_to_add, false);
        }
        void reserve_map_at_front(size_type nodes_to_add = 1) {
            if(nodes_to_add > start.node - map)
                reallocate_map(nodes_to_add, true);
        }
        void reallocate_map(size_type nodes_to_add, bool add_at_front);
    public:
        iterator begin() {
            return start;
        }
        iterator end() {
            return finish;
        }
        reference operator[] (size_type n) {
            return start[difference_type(n)];
        }
        reference front() {
            return *start;
        }
        reference back() {
            iterator tmp = finish;
            --tmp;
            return *tmp;
        }
        size_type size() const {
            return finish - start;
        }
        size_type max_size() const {
            return size_type(-1);
        }
        bool empty() const {
            return finish == start;
        }
        void fill_initialize(size_type n, const value_type& value);
        void create_map_and_nodes(size_type num_elements);
        deque(int n, const value_type& value) : start(), finish(), map(0), map_size(0) {
            fill_initialize(n, value);
        }
        void push_back(const value_type& t) {
            if(finish.cur != finish.last - 1) {
                construct(finish.cur, t);
                ++finish.cur;
            }else {
                push_back_aux(t);
            }
        }
        void push_front(const value_type& t){
            if(start.cur != start.first) {
                construct(start.cur - 1, t);
                --start.cur;
            }else {
                push_front_aux(t);
            }
        }
        void pop_back() {
            if(finish.cur != finish.first) {
                --finish.cur;
                destroy(finish.cur);
            }else {
                pop_back_aux();
            }
        }
        void pop_front() {
            if(start.cur != start.last - 1) {
                destroy(start.cur);
                ++start.cur;
            }else {
                pop_front_aux();
            }
        }
        void clear() {
            for(map_pointer node = start.node + 1; node < finish.node; ++node) {
                destroy(*node, *node + buffer_size());
                data_allocator::deallocate(*node, buffer_size());
            }
            if(start.node != finish.node) {
                destroy(start.cur, start.last);
                destroy(finish.first, finish.cur);
                data_allocator::deallocate(finish.first, buffer_size());
            }else
                destroy(start.cur, finish.cur);
            finish = start;
        }
        iterator erase(iterator pos) {
            iterator next = pos;
            ++next;
            difference_type index = pos - start; //清除点之前的元素个数
            if(index < (size() >> 1)) {   //如果清除点之前的元素比较少
                copy_backward(start, pos, next);
                pop_front();
            }else {
                copy(next, finish, pos);
                pop_back();
            }
            return start + index;
        }
        iterator erase(iterator first, iterator last) {
            if(first == start && last == finish) {
                clear();
                return finish;
            }else {
                difference_type n = last - first;
                difference_type elems_before = first - start;
                if(elems_before < (size() - n) / 2) {
                    copy_backward(start, first, last);
                    iterator new_start = start + n;
                    destroy(start, new_start);
                    for(map_pointer cur = start.node; cur < new_start.node; ++cur)
                        data_allocator::deallocate(*cur, buffer_size());
                    start = new_start;
                }else {
                    copy(last, finish, first);
                    iterator new_finish = finish - n;
                    destroy(new_finish, finish);
                    for(map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
                        data_allocator::deallocate(*cur, buffer_size());
                    finish = new_finish;
                }
                return start + elems_before;
            }
        }
        iterator insert(iterator position, const vaule_type& x) {
            if(position.cur == start.cur) {
                push_front(x);
                return start;
            }else if(position.cur == finish.cur) {
                push_back(x);
                iterator tmp = finish;
                --tmp;
                return tmp;
            }else {
                return insert_aux(position, x);
            }
        }
    };
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements) {
        //需要节点数=（元素个数除以每个缓冲区可容纳的个数）+ 1
        size_type num_nodes = num_elements / buffer_size() + 1;
        //一个map最少管理8个节点，最多是所需节点数加2
        //前后各留一个备用
        map_size = max(initial_map_size(), num_nodes + 2);
        map = map_allocator::allocate(map_size);
        map_pointer nstart = map + (map_size - num_nodes) / 2;
        map_pointer nfinish = nstart + num_nodes - 1;
        map_pointer cur;
        for(cur = nstart; cur <= nfinish; ++cur)
            *cur = allocate_node();
        start.set_node(nstart);
        finish.set_node(nfinish);
        start.cur = start.first;
        finish.cur = finish.first + num_elements % buffer_size();

    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::fill_initialize(size_type n, const value_type& value) {
        create_map_and_nodes(n);
        map_pointer cur;
        for(cur = start.node; *cur < finish.node; ++cur)
            uninitialized_fill(*cur, *cur + buffer_size(), value);
        uninitialized_fill(finish.first, finish.cur, value);
    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::push_back_aux(const value_type &t) {
        value_type t_copy = t;
        reserve_map_at_back();  //若符合某种条件则必须重换一个map
        *(finish.node + 1) = allocate_node(); //配置一个新节点
        construct(finish.cur, t_copy);
        finish.set_node(finish.node + 1);
        finish.cur = finish.first;
    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::push_front_aux(const value_type &t) {
        value_type t_copy = t;
        reserve_map_at_front();
        *(start.node - 1) = allocate_node();
        start.set_node(start.node - 1);
        start.cur = start.last - 1;
        construct(start.cur, t_copy);
    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add, bool add_at_front) {
        size_type old_num_nodes = finish.node - start.node + 1;
        size_type new_num_nodes = old_num_nodes + nodes_to_add;
        map_pointer new_nstart;
        if(map_size > 2 * new_num_nodes) {
            new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            if(new_nstart < start.node)
                copy(start.node, finish.node + 1, new_nstart);
            else
                copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
        }else {
            size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
            map_pointer new_map = map_allocator::allocate(new_map_size);
            new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            copy(start.node, finish.node + 1, new_nstart);
            map_allocator::deallocate(map, map_size);
            map = new_map;
            map_size = new_map_size;
        }
        start.set_node(new_nstart);
        finish.set_node(new_nstart + old_num_nodes - 1);
    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::pop_back_aux() {
        deallocate_node(finish.first);
        finish.set_node(finish.node - 1);
        finish.cur = finish.last - 1;
        destroy(finish.cur);
    }
    template <class T, class Alloc, size_t BufSize>
    void deque<T, Alloc, BufSize>::pop_front_aux() {
        destroy(start.cur);
        deallocate_node(start.first);
        start.set_node(start.node + 1);
        start.cur = start.first;
    }
    template <class T, class Alloc, size_t BufSize>
    iterator deque<T, Alloc, BufSize>::insert_aux(iterator pos, const int &x) {
        difference_type index = pos - start;
        value_type x_copy = x;
        if(index < (size() / 2)) {
            push_front(front());
            iterator front1 = start;
            ++front1;
            iterator front2 = front1;
            ++front2;
            pos = start + index;
            iterator pos1 = pos;
            ++pos1;
            copy(front2, pos1, front1);

        }else {
            push_back(back());
            iterator back1 = finish;
            --back1;
            iterator back2 = back1;
            --back2;
            pos = start + index;
            copy_backward(pos, back2, back1);

        }
        *pos = x_copy;
        return pos;
    }
}
#endif //MY_STL_DEQUE_H
