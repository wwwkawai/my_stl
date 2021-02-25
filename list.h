#ifndef MY_STL_LIST_H
#define MY_STL_LIST_H
#include "iterator.h"
#include "alloc.h"
#include "cons.h"
#include "unin.h"
namespace my_stl{
    template <class T>
    struct _list_node{
        T data;
        typedef void* void_pointer;
        void_pointer prev;
        void_pointer next;
    };
    //迭代器
    template <class T, class Ref, class Ptr>
    struct _list_iterator {
        typedef _list_iterator<T, T&, T*> iterator;
        typedef _list_iterator<T, Ref, Ptr> self;
        typedef bidirectional_iterator_tag iterator_catrgory;
        typedef T value_type;
        typedef Ptr pointer;
        typedef Ref reference;
        typedef _list_node<T>* link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        link_type node; //指向list的节点

        //constructor
        _list_iterator(link_type x) :node(x) {}
        _list_iterator() {}
        _list_iterator(const iterator& x) : node(x.node) {}

        bool operator== (const self& x) const { return node == x.node }
        bool operator!= (const self& x) const { return node != x.node }
        //取节点的值
        reference operator*() const { return (*node).data; }

        //对迭代器的成员存取（member access）运算子的标准做法
        pointer operator->() const { return &(operator*()); }
        //递增
        self& operator++() {
            node = (link_type) ((*node).next);
            return *this;
        }
        self operator++(int) {
            self tmp = *this;
            ++*this;
            return tmp;
        }
        //递减
        self& operator--() {
            node = (link_type) ((*node).prev);
            return *this;
        }
        self operator--(int) {
            self tmp = *this;
            --*this;
            return tmp;
        }
    };
    template <class T, class Alloc = alloc>
    class list {
    protected:
        typedef _list_node<T> list_node;
        typedef my_alloc<list_node, Alloc> list_node_allocator; //专属空间配置器，每次配置一个节点
    public:
        typedef list_node* link_type;
        typedef T value_type;
        typedef value_type& reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef _list_iterator<T, T&, T*> iterator;
    protected:
        link_type node; //让node指向刻意置于尾端的一个空白节点
        //配置一个节点并返回
        link_type get_node() { return list_node_allocator::allocate(); }
        //释放一个节点
        void put_node(link_type p) { list_node_allocator::deallocate(p); }

        //产生一个节点，带有元素值
        link_type create_node(const T& x) {
            link_type p = get_node();
            construct(&p->data, x);
            return p;
        }
        //销毁一个节点
        void destroy_node(link_type p) {
            destroy(&p->data);
            put_node(p);
        }
        void empty_initialize() {
            node = get_node();
            node->next = node;
            node->prev = node;
        }
    public:
        list() { empty_initialize(); } //产生一个空链表
        iterator begin() { return (link_type)((*node).next); }
        iterator end() { return node; }
        bool empty() { return node->next == node; }
        size_type size() const {
            size_type result = 0;
            distance(begin(), end(), result);
            return result;
        }
        reference front() { return *begin(); }
        reference back() { return *(--end()); }
        iterator insert(iterator position, const T& x) {
            link_type tmp = create_node(x);
            tmp->next = position.node;
            tmp->prev = position.node->prev;
            (link_type(position.node->prev))->next = tmp;
            position.node->prev = tmp;
            return tmp;
        }
        void push_front(const T& x) { insert(begin(), x); }
        void push_back(const T& x) { insert(end(), x); }
        iterator erase(iterator position) {
            link_type next_node = link_type(position.node->next);
            link_type prev_node = link_type(position.node->prev);
            prev_node->next = next_node;
            next_node->prev = prev_node;
            destroy_node(position.node);
            return iterator(next_node);
        }
        void pop_front() {
            erase(begin());
        }
        void pop_back() {
            iterator tmp = end();
            erase(--tmp);
        }
        void clear() {
            link_type cur = begin();
            while(cur != end()){
                link_type tmp = cur;
                cur = (link_type) cur->next;
                destroy_node(tmp);
            }
        }
        void remove(const T& value) { //移除所有值为value的元素
            iterator first = begin() ;
            iterator last = end();
            while(first != last){
                iterator next = first;
                ++next;
                if(*first == value) erase(first);
                first = next;
            }
        }
        //移除额外的数值相同的连续元素
        void unique() {
            iterator first = begin();
            iterator last = end();
            if(first == last) return;
            iterator next = first;
            while(++next != last) {
                if(*first == *next)
                    erase(next);
                else
                    first = next;
                next = first;
            }
        }
    protected:
        //将[first，last)内的所有元素移到position之前
        void transfer(iterator position, iterator first, iterator last) {
            if(position != last){
                (*(link_type((*last.node).prev))).next = position.node;
                (*(link_type((*first.node).prev))).next = last.node;
                (*(link_type((*position.node).prev))).next = first.node;
                link_type tmp = link_type((*position.node).prev);
                (*position.node).prev = (*last.node).prev;
                (*last.node).prev = (*first.node).prev;
                (*first.node).prev = tmp;
            }
        }

    public:
        //x必须不同于*this
        void splice(iterator postion, list& x) {
            if(!x.empty())
                transfer(postion, x.begin(), x.end());
        }
        //将i所指元素接合于position所指位置之前,position和i可指向同一个list
        void splice(iterator position, list&, iterator i) {
            iterator j = i;
            ++j;
            if(position == i || position == j) return;
            transfer(position, i, j);
        }
        //将[first，last)内的所有元素移到position之前,position和该区间可位于同一list，但不能重合
        void splice(iterator position, list&, iterator first, iterator last) {
            if(first != last)
                transfer(position, first, last);
        }
        //merge()将x合并在*this上，必须是单调不减的
        void merge(list<T, Alloc>& x) {
            iterator first1 = begin();
            iterator last1 = end();
            iterator first2 = x.begin();
            iterator last2 = x.end();
            while(first1! = last1 && first2 != last2){
                if(*first1 < *first2) {
                    iterator next = first2;
                    transfer(first1, first2, ++next);
                    first2 = next;
                }else{
                    ++first1;
                }
            }
            if(first2 != last2)
                transfer(last1, first2, last2);
        }
        //逆置
        void reverse() {
            if(node->next == node || link_type(node->next)->next == node) return;
            iterator first = begin();
            ++first;
            while(first != end()){
                iterator old = first;
                ++first;
                transfer(begin(), old, first);
            }
        }
        void swap(list<T, Alloc>& x) {
            my_stl::swap(node, x.node);
        }
        //list 不能直接用STL的sort()
        void sort() {
            if(node->next == node || link_type(node->next)->next == node) return;
            //中介数据存放区
            list<T, Alloc> carry;
            list<T, Alloc> counter[64];
            int fill = 0;
            while(!empty()) {
                carry.splice(carry.begin(), *this, begin());
                int i = 0;
                while(i < fill && !counter[i].empty()){
                    counter[i].merge(carry);
                    carry.swap(counter[i++]);
                }
                carry.swap(counter[i]);
                if(i == fill) ++fill;
            }
            for(int i = 1; i < fill; ++i)
                counter[i].merge(counter[i-1]);
            swap(counter[fill-1]);
        }
    };
}
#endif //MY_STL_LIST_H
