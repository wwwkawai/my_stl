#ifndef MY_STL_RB_TREE_H
#define MY_STL_RB_TREE_H
#include "iterator.h"
#include "alloc.h"
#include "unin.h"
namespace my_stl{
    typedef bool _rb_tree_color_type;
    const _rb_tree_color_type _rb_tree_red = false;
    const _rb_tree_color_type _rb_tree_black = true;
    struct _rb_tree_node_base{
        typedef _rb_tree_color_type color_type;
        typedef _rb_tree_node_base* base_ptr;
        color_type color; //节点颜色
        base_ptr parent;
        base_ptr left;
        base_ptr right;
        static base_ptr minimum(base_ptr x){
            while(x->left != 0)
                x = x -> left;
            return x;
        }
        static base_ptr maximum(base_ptr x){
            while(x -> right != 0)
                x = x -> right;
            return x;
        }
    };
    template <class Value>
    struct _rb_tree_node : public _rb_tree_node_base{
        typedef _rb_tree_node<Value>* link_type;
        Value value_field; //节点值
    };
    //双层迭代器
    struct _rb_tree_base_iterator{
        typedef _rb_tree_node_base::base_ptr base_ptr;
        typedef bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        base_ptr node; //产生连接关系
        void increment() {
            if(node -> right != 0){
                node = node -> right;
                while(node -> left != 0)
                    node = node -> left;
            }else{
                base_ptr y = node -> parent;
                while(node == y -> right){
                    node = y;
                    y = y -> parent;
                }
                if(node -> right != y){
                    node = y;
                }
            }
        }
        void decrement() {
            if(node -> color == _rb_tree_red && node -> parent -> parent == node)
                node = node -> right;
            else if(node -> left != 0){
                base_ptr y = node -> left;
                while(y -> right != 0)
                    y = y -> right;
                node = y;
            }else{
                base_ptr y = node -> parent;
                while(node == y -> left){
                    node = y;
                    y = y -> parent;
                }
                node = y;
            }
        }
    };
    template <class Value, class Ref, class Ptr>
    struct _rb_tree_iterator : public _rb_tree_base_iterator{
        typedef Value value_type;
        typedef Ref reference;
        typedef Ptr pointer;
        typedef _rb_tree_iterator<Value, Value&, Value*> iterator;
        typedef _rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
        typedef _rb_tree_iterator<Value, Ref, Ptr> self;
        typedef _rb_tree_node<Value>* link_type;
        _rb_tree_iterator() {}
        _rb_tree_iterator(link_type x) {
            node = x;
        }
        _rb_tree_iterator(const iterator& it) {
            node = it.node;
        }
        reference operator* () const {
            return link_type(node) -> value_field;
        }
        pointer operator-> () const {
            return &(operator*());
        }
        self& operator++ () {
            increment();
            return *this;
        }
        self operator++ (int) {
            self tmp = *this;
            increment();
            return tmp;
        }
        self& operator-- () {
            decrement();
            return *this;
        }
        self operator-- (int) {
            self tmp = *this;
            decrement();
            return tmp;
        }
    };
    template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = alloc>
    class rb_tree{
    protected:
        typedef void* void_pointer;
        typedef _rb_tree_node_base* base_ptr;
        typedef _rb_tree_node<Value> rb_tree_node;
        typedef my_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
        typedef _rb_tree_color_type color_type;
    public:
        typedef Key Key_type;
        typedef Value value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef rb_tree_node* link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
    protected:
        link_type get_node() {
            return rb_tree_node_allocator::allocate();
        }
        link_type create_node(const value_type& x) {
            link_type tmp = get_node();
            construct(&tmp -> value_field, x);
            return tmp;
        }
        link_type clone_node(link_type x){
            link_type tmp = create_node(x -> value_field);
            tmp -> color = x->color;
            tmp -> left = 0;
            tmp -> right = 0;
            return tmp;
        }
        void destroy_node(link_type p) {
            destroy(&p -> value_field);
            put_node(p);
        }

    protected:
        size_type node_count;  //节点数量
        link_type header; //实现技巧
        Compare key_compare; //function object;
        //取header的成员
        link_type& root() const {
            return (link_type&) header -> parent;
        }
        link_type& leftmost() const {
            return (link_type&) header -> left;
        }
        link_type& right_most() const {
            return (link_type&) header -> right;
        }
        //取节点x的成员
        static link_type& left(link_type x) {
            return (link_type&) (x -> left);
        }
        static link_type& right(link_type x) {
            return (link_type&) (x -> right);
        }
        static link_type& parent(link_type x) {
            return (link_type&) (x -> parent);
        }
        static reference value(link_type x) {
            return x -> value_field;
        }
        static const Key& key(link_type x) {
            return KeyOfValue() (value(x));
        }
        static color_type& color(link_type x) {
            return (color_type&) (x -> color);
        }



        static link_type& left(base_ptr x) {
            return (link_type&) (x -> left);
        }
        static link_type& right(base_ptr x) {
            return (link_type&) (x -> right);
        }
        static link_type& parent(base_ptr x) {
            return (link_type&) (x -> parent);
        }
        static reference value(base_ptr x) {
            return (link_type)x -> value_field;
        }
        static const Key& key(base_ptr x) {
            return KeyOfValue() (value(x));
        }
        static color_type& color(base_ptr x) {
            return (color_type&) (x -> color);
        }
        static link_type minimum(link_type x){
            return (link_type) _rb_tree_node_base::minimum(x);
        }
        static link_type maxmum(link_type x){
            return (link_type) _rb_tree_node_base::maximum(x);
        }
    public:
        typedef _rb_tree_iterator<value_type, reference, pointer> iterator;
    private:
        iterator _insert(base_ptr x, base_ptr y, const value_type& v);
        iterator find(const Key_type& k);
        link_type _copy(link_type x ,link_type p);
        void _erase(link_type x);
        void init(){
            header = get_node();
            color(header) = _rb_tree_red;
            root() = 0;
            leftmost() = header;
            right_most() = header;
        }
    public:
        rb_tree(const Compare& comp = Compare()): node_count(0),key_compare(comp){
            init();
        }
        ~rb_tree(){
            clear();
            put_node(header);
        }
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& operator = (const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);
    public:
        Compare key_comp() const {
            return key_compare;
        }
        iterator begin(){
            return leftmost();
        }
        iterator end(){
            return right_most();
        }
        bool empty() const{
            return node_count==0;
        }
        size_type size() const {
            return node_count;
        }
        size_type max_size() const {
            return size_type(-1);
        }
    public:
        pair<iterator,bool> insert_unique(const value_type& v){
            link_type y = header;
            link_type x = root();
            bool comp = true;
            while(x!=0){
                y=x;
                comp = key_compare(KeyOfValue()(v), key(x));
                x = comp? left(x): right(x);
            }
            iterator j = iterator(y);
            if(comp){
                if(j==begin()){
                    return pair<iterator,bool>(_insert(x,y,v),true);
                }else{
                    --j;
                }
            }
            if(key_compare(ke(j.node),KeyOfValue()(v))){
                return pair<iterator,bool>(_insert(x,y,v),true);
            }
            return pair<iterator,bool>(j,false);
        }
        iterator insert_equal(const value_type& v){
            link_type y = header;
            link_type x = root();
            while(x!=0){
                y = x;
                x = key_compare(KeyOfValue()(v), key(x))? left(x): right(x);
            }
            return _insert(x,y,v);

        }
        inline void _rb_tree_rotate_left(_rb_tree_node_base* x, _rb_tree_node_base*& root){
            _rb_tree_node_base*y = x->right;
            x->right = y->left;
            if(y->left!=0)
                y->left->parent = x;
            y->parent = x->parent;
            if(x==root)
                root=y;
            else if(x==x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;
            y->left = x;
            x->parent = y;
        }
        inline void _rb_tree_rotate_right(_rb_tree_node_base* x, _rb_tree_node_base*& root){
            _rb_tree_node_base* y = x->left;
            x->left = y->right;
            if(y->right!=0)
                y->right->parent = x;
            y->parent = x->parent;
            if(x==root)
                root=y;
            else if(x==x->parent->right)
                x->parent->right = y;
            else
                x->parent->left = y;
            y->right = x;
            x->parent = y;

        }

        inline void _rb_tree_rebalance(_rb_tree_node_base* x, _rb_tree_node_base*& root) {
            x->color = _rb_tree_red;
            while (x != root && x->parent->color == _rb_tree_red) {
                if (x->parent == x->parent->parent->left) {
                    _rb_tree_node_base *y = x->parent->parent->right;
                    if (y && y->color == _rb_tree_red) {
                        x->parent->color = _rb_tree_black;
                        y->color = _rb_tree_black;
                        x->parent->parent->color = _rb_tree_red;
                        x = x->parent->parent;
                    } else {
                        if (x == x->parent->right) {
                            x = x->parent;
                            _rb_tree_rotate_left(x, root);
                        }
                        x->parent->color = _rb_tree_black;
                        x->parent->parent->color = _rb_tree_red;
                        _rb_tree_rotate_right(x->parent->parent, root);
                    }
                }else{
                    _rb_tree_node_base* y = x->parent->parent->left;
                    if(y&&y->color==_rb_tree_red) {
                        x->parent->color = _rb_tree_black;
                        y->color = _rb_tree_black;
                        x->parent->parent->color = _rb_tree_red;
                        x = x->parent->parent;
                    }else{
                        if(x==x->parent->left){
                            x=x->parent;
                            _rb_tree_rotate_right(x, root);
                        }
                        x->parent->color=_rb_tree_black;
                        x->parent->parent->color=_rb_tree_red;
                        _rb_tree_rotate_left(x->parent->parent, root);
                    }
                }

            }
            root->color = _rb_tree_black;
        }
    };
    template<class Key, class Value, class KeyofValue, class Compare,class Alloc>
    typename rb_tree<Key, Value, KeyofValue, Compare, Alloc>::iterator
    rb_tree<Key, Value, KeyofValue, Compare, Alloc>::
            _insert(base_ptr x_, base_ptr y_, const value_type &v) {
        link_type x = (link_type) x_;
        link_type y = (link_type) y_;
        link_type z;
        if(y==header||x!=0||key_compare(KeyofValue()(v),key(y))){
            z = create_node(v);
            left(y) = z;
            if(y==header){
                root() = z;
                right_most() = z;
            }else if(y==leftmost()){
                leftmost() = z;
            }
        }else{
            z = create_node(v);
            right(y) = z;
            if(y==right_most()){
                right_most() = z;
            }
        }
        parent(z) = y;
        left(z) = 0;
        right(z) = 0;
        _rb_tree_rebalance(z,header->parent);
        ++node_count;
        return iterator(z);
    }
    template<class Key, class Value, class KeyofValue, class Compare,class Alloc>
    typename rb_tree<Key, Value, KeyofValue, Compare, Alloc>::iterator
            rb_tree<Key, Value, KeyofValue, Compare, Alloc>::find(const Key& k){
                link_type y = header;
                link_type x = root();
                while(x!=0)
                    if(!key_compare(key(x),k)){
                        y=x;
                        x= left(x);
                    }else{
                        x= right(x);
                    }
                iterator j = iterator(y);
                return (j==end()||key_compare(k,key(j.node)))?end():j;
            }
}
#endif //MY_STL_RB_TREE_H
