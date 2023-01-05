
#ifndef MY_STL_HASHTABLE_H
#define MY_STL_HASHTABLE_H
#include "iterator.h"
#include "alloc.h"
#include "vector.h"
namespace my_stl{
    template<class Value>
    struct _hashtable_node{
        _hashtable_node* next;
        Value val;
    };

    template<class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>
    struct _hashtable_iterator;

    template<class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc = alloc>
    class hashtable{
    public:
        typedef HashFcn hasher;
        typedef EqualKey key_equal;
        typedef size_t size_type;
    private:
        hasher hash;
        key_equal equals;
        ExtractKey get_key;
        typedef _hashtable_node<Value> node;
        typedef my_alloc<node,Alloc> node_allocator;
        typedef _hashtable_iterator<Value,Key,HashFcn,ExtractKey,EqualKey,Alloc> iterator;
        vector<node*,Alloc> buckets;
        size_type num_elements;
        typedef Value value_type;
        typedef Key key_type;
        static const int _stl_num_prime = 28;
        constexpr static const unsigned long _stl_prime_list[_stl_num_prime]={
                53,97,193,389,769,
                1543,3079,6151,12289,24593,
                49157,98317,196613,393241,786433,
                15272869,3145739,6291469,12582917,25165843,
                50331653,100663319,201326611,402653189,805306457,
                1610612741,3221225473ul,4294967291ul
        };
        inline unsigned long _stl_next_prime(unsigned long n){
            const unsigned long* first = _stl_prime_list;
            const unsigned long* last = _stl_prime_list+_stl_num_prime;
            const unsigned long* pos =lower_bound(first, last, n);
            return pos == last? *(last-1):*pos;
        }
        size_type max_bucket_count() const{
            return _stl_prime_list[_stl_num_prime-1];
        }
    public:
        size_type bucket_count() const{
            return buckets.size();
        }

    private:
        node* new_node(const value_type& obj){
            node* n = node_allocator::allocate();
            n->next=0;
            construct(&n->val,obj);
            return n;
        }
        void delete_node(node* n){
            destroy(&n->val);
            node_allocator::deallocate(n);
        }
        void initialize_buckets(size_type n){
            const size_type n_buckets = next_size(n);
            buckets.reserve(n_buckets);
            buckets.insert(buckets.end(),n_buckets,(node*)0);
            num_elements = 0;
        }
        size_type next_size(size_type n) const{
            return _stl_next_prime(n);
        }
        pair<iterator,bool> insert_unique(const value_type& obj){
            resize(num_elements+1);
            return insert_unique_noresize(obj);
        }
        void resize(size_type num_elements_hint){
            const size_type old_n = buckets.size();
            if(num_elements_hint>old_n) {
                const size_type n = next_size(num_elements_hint);
                if(n>old_n){
                    vector<node*, Alloc> tmp(n,(node*)0);
                    for(size_type bucket=0;bucket<old_n;++bucket){
                        node* first = buckets[bucket];
                        while(first){
                            size_type new_bucket = bkt_num(first->val,n);
                            buckets[bucket] = first->next;
                            first->next = tmp[new_bucket];
                            tmp[new_bucket] = first;
                            first = buckets[bucket];
                        }
                    }
                    buckets.swap(tmp);
                }
            }

        }
        pair<iterator,bool> insert_unique_noresize(const value_type& obj){
            const size_type n = bkt_num(obj);
            node* first = buckets[n];
            for(node* cur = first; cur; cur=cur->next){
                if(equals(get_key(cur->val),get_key(obj)))
                    return pair<iterator,bool>(iterator(cur,this), false);

            }
            node* tmp = new_node(obj);
            tmp->next = first;
            buckets[n]=tmp;
            ++num_elements;
            return pair<iterator,bool>(iterator(tmp,this), true);
        }
        iterator insert_equal(const value_type& obj){
            resize(num_elements+1);
            return insert_equal_noresize(obj);
        }
        iterator insert_equal_noresize(const value_type& obj) {
            const size_type n = bkt_num(obj);
            node *first = buckets[n];
            for (node *cur = first; cur; cur = cur->next) {
                if(equals(get_key(cur->val),get_key(obj))){
                    node* tmp = new_node(obj);
                    tmp->next = cur->next;
                    cur->next = tmp;
                    ++num_elements;
                    return iterator(tmp,this);
                }
            }
            node* tmp = new_node(obj);
            tmp->next = first;
            buckets[n]=tmp;
            ++num_elements;
            return iterator(tmp,this);
        }
        size_type bkt_num(const value_type& obj,size_t n) const{
            return bkt_num_key(get_key(obj),n);
        }
        void clear() {
            for (size_type i = 0; i < buckets.size(); ++i) {
                node* cur = buckets[i];
                while(cur!=0){
                    node* next = cur->next;
                    delete_node(cur);
                    cur = next;
                }
                buckets[i]=0;
            }
            num_elements = 0;
        }
        void copy_from(const hashtable& ht) {
            buckets.clear();
            buckets.reserve(ht.buckets.size());
            buckets.insert(buckets.end(), ht.buckets.size(), (node *) 0);
            for (size_type i = 0; i < ht.buckets.size(); ++i) {
                if (const node *cur = ht.buckets[i]) {
                    node *copy = new_node(cur->val);
                    buckets[i] = copy;
                    for (node *next = cur->next; next; cur = next, next = cur->next) {
                        copy->next = new_node(next->val);
                        copy = copy->next;
                    }
                }

            }
            num_elements = ht.num_elements;
        }
    public:
        iterator find(const key_type& key){
            size_type n = bkt_num_key(key);
            node* first;
            for(first = buckets[n];first&&!equals(get_key(first->val),key);first = first->next){
            }
            return iterator(first,this);
        }
        size_type count(const key_type& key) const {
            const size_type n = bkt_num_key(key);
            size_type result = 0;
            for (const node *cur = buckets[n]; cur; cur = cur->next) {
                if (equals(get_key(cur->val), key))
                    ++result;
            }
            return result;
        }
    };


    template<class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>
    struct _hashtable_iterator{
        typedef hashtable<Value,Key,HashFcn,ExtractKey,EqualKey,Alloc> hashtable;
        typedef _hashtable_iterator<Value,Key,HashFcn,ExtractKey,EqualKey,Alloc> iterator;
        typedef _hashtable_node<Value> node;
        typedef Value& reference;
        typedef Value* pointer;
        typedef size_t size_type;
        typedef ptrdiff_t different_type;
        typedef forward_iterator_tag iterator_category;
        node* cur;
        hashtable* ht;
        _hashtable_iterator(node* n, hashtable* tab):cur(n),ht(tab){}
        _hashtable_iterator(){}
        reference operator*() const {
            return cur->val;
        }
        pointer operator ->() const {
            return &(operator*());
        }
        iterator& operator++(){
            const node* old = cur;
            cur = cur->next;
            if(!cur){

            }
            return *this;
        }
        iterator operator++(int){
            iterator tmp = *this;
            ++*this;
            return tmp;
        }
        bool operator==(const iterator& it) const {
            return cur==it.cur;
        }
        bool operator!=(const iterator& it) const{
            return cur!=it.cur;
        }
    };

}



#endif //MY_STL_HASHTABLE_H
