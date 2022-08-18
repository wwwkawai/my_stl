#ifndef MY_STL_ALLOC_H
#define MY_STL_ALLOC_H
#include <new>
#include <cstddef>
#include <stdlib.h>
#include <cstdio>
namespace my_stl {

    template<int inst>
    class _malloc_alloc_template {
    private:
        //函数指针，代表处理内存不足的函数 oom:out of memory
        static void *oom_malloc(size_t);

        static void *oom_realloc(void *, size_t);

        static void (*_malloc_alloc_oom_handler)();

    public:
        static void *allocate(size_t n) {
            void *result = malloc(n); //第一级配置使用malloc()
            if (0 == result) result = oom_malloc(n);//不成功则用oom_malloc()
            return result;
        }

        static void *deallocate(void *p, size_t n) {
            free(p); //第一级配置使用free();
        }

        static void *reallocate(void *p, size_t old_size, size_t new_size) {
            void *result = realloc(p, new_size); //第一级配置直接使用realloc();
            if (0 == result) result = oom_realloc(p, new_size); //不成功时使用oom_realloc();
            return result;
        }

        //仿真c++的set_new_handler(),可以通过它指定自己的oom handler
        static void (*set_malloc_handler(void (*f)()))() {
            void (*old)() = _malloc_alloc_oom_handler;
            _malloc_alloc_oom_handler = f;
            return old;
        }
    };
    typedef _malloc_alloc_template<0> malloc_alloc;
//malloc_alloc oom handling 初值为0,待客端设定
    template<int inst>
    void (*_malloc_alloc_template<inst>::_malloc_alloc_oom_handler)() = 0;

    template<int inst>
    void *_malloc_alloc_template<inst>::oom_malloc(size_t n) {
        void (*my_malloc_handler)();
        void *result;
        for (;;) { // 不断尝试释放，配置，再释放，再配置
            my_malloc_handler = _malloc_alloc_oom_handler;
            if (0 == my_malloc_handler) { throw std::bad_alloc(); }
            (*my_malloc_handler)();  //调用处理例程，试图释放内存
            result = malloc(n);  // 再次尝试配置内存
            if (result) return (result);
        }
    }

    template<int inst>
    void *_malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
        void (*my_malloc_handler)();
        void *result;
        for (;;) { // 不断尝试释放，配置，再释放，再配置
            my_malloc_handler = _malloc_alloc_oom_handler;
            if (0 == my_malloc_handler) { throw std::bad_alloc(); }
            (*my_malloc_handler)(); //调用处理例程，试图释放内存
            result = realloc(p, n); // 再次尝试配置内存
            if (result) return (result);
        }
    }

// 单纯地转调用，调用传递给配置器(第一级或第二级)；多一层包装，使 Alloc 具备标准接口
    template<class T, class Alloc>
    class my_alloc {
    public:
        static T *allocate(size_t n) {
            return 0 == n ? 0 : (T *) Alloc::allocate(n * sizeof(T));
        }

        static T *allocate() {
            return (T *) Alloc::allocate(sizeof(T));
        }

        static void deallocate(T *p, size_t n) {
            if (0 != n)
                Alloc::deallocate(p, n * sizeof(T));
        }

        static void deallocate(T *p) {
            Alloc::deallocate(p, sizeof(T));
        }
    };

    enum {
        _align = 8
    }; //小型区块的上调边界
    enum {
        _max_bytes = 128
    }; //小型区块的上界
    enum {
        _nfreelists = _max_bytes / _align
    }; //free_lists个数
// 第二级配置器
//不讨论多线程
    template<int inst>
    class _default_alloc_template {
    private:
        union block {
            union block *free_list_link;
            char client_data[1]; // 客端可见
        };
    private:
        //round_up() 将bytes上调为8的倍数
        static size_t round_up(size_t bytes) {
            return (((bytes) + _align - 1) & ~(_align - 1)); //加7去余
        }

    private:
        //16个free_lists
        static block *volatile free_list[_nfreelists];

        //根据区块大小，选择第n个free_list
        static size_t freelist_index(size_t bytes) {
            return (((bytes) + _align - 1) / _align - 1);
        }

        //返回一个大小为n的对象，并可能加入大小为n的其它区块到free_list
        static void *refill(size_t n);

        //配置一块可容纳nobjs个大小为size的区块，若不能满足，nobjs可能会降低
        static char *chunk_alloc(size_t size, int &nobjs);

        //chunk allocation state
        //内存池开始结束位置，只在chunk_alloc()中变化
        static char *start_free;
        static char *end_free;

        static size_t heap_size;
    public:
        static void *allocate(size_t n) {
            block *volatile *my_free_list;
            block *result;
            //n>128 使用malloc_alloc
            if (n > (size_t) _max_bytes) {
                return (malloc_alloc::allocate(n));
            }
            //寻找16个free_list中适当的一个
            my_free_list = free_list + freelist_index(n);
            result = *my_free_list;
            if (0 == result) {
                //无可用的free_list，准备refill
                void *r = refill(round_up(n));
                return r;
            }
            //调整free_list
            *my_free_list = result->free_list_link;
            return (result);
        }

        static void deallocate(void *p, size_t n) {
            block *q = (block *) p;
            block *volatile *my_free_list;
            //n>128使用第一级配置器
            if (n > (size_t) _max_bytes) {
                malloc_alloc::deallocate(p, n);
                return;
            }
            //寻找对应free_list
            my_free_list = free_list + freelist_index(round_up(n));
            //调整free_list,回收区块
            q->free_list_link = *my_free_list;
            *my_free_list = q;
        }

        static void *reallocate(void *p, size_t old_size, size_t new_size);
    };
    typedef _default_alloc_template<0> alloc;
//设置初值
    template<int inst>
    char *_default_alloc_template<inst>::start_free = 0;
    template<int inst>
    char *_default_alloc_template<inst>::end_free = 0;
    template<int inst>
    size_t _default_alloc_template<inst>::heap_size = 0;
    template<int inst>
    typename  _default_alloc_template<inst>::block *volatile _default_alloc_template<inst>::free_list[_nfreelists] = {0, 0, 0, 0,
                                                                                                            0, 0, 0, 0,
                                                                                                            0, 0, 0, 0,
                                                                                                            0, 0, 0, 0};

    template<int inst>
    void *_default_alloc_template<inst>::refill(size_t n) {
        int nobjs = 20;
        //调用chunk_alloc()，尝试取得nobjs个区块作为free_list的新节点
        char *chunk = chunk_alloc(n, nobjs);
        block *volatile *my_free_list;
        block *result;
        block *current_block, *next_block;
        int i;
        //如果只有一个区块直接返回给调用者，free_list无新节点
        if (1 == nobjs) return (chunk);
        //否则调整free_list
        my_free_list = free_list + freelist_index(n);
        //在chunk空间内建立free_list
        result = (block *) chunk;  //这一块返回给客端
        //导引free_list指向新空间 （取自内存池）
        *my_free_list = next_block = (block *) (chunk + n);
        for (i = 1;; i++) {
            current_block = next_block;
            next_block = (block *) ((char *) next_block + n);
            if (nobjs - 1 == i) {
                current_block->free_list_link = 0;
                break;
            } else {
                current_block->free_list_link = next_block;
            }
        }
        return (result);
    }

    template<int inst>
    char *_default_alloc_template<inst>::chunk_alloc(size_t size, int &nobjs) {
        char *result;
        size_t total_bytes = size * nobjs;
        size_t bytes_left = end_free - start_free; // 内存池剩余空间
        if (bytes_left >= total_bytes) {
            //可满足
            result = start_free;
            start_free += total_bytes;
            return (result);
        } else if (bytes_left >= size) {
            //不能完全满足，但足够供应一个以上的区块
            nobjs = bytes_left / size;
            total_bytes = nobjs * size;
            result = start_free;
            start_free += total_bytes;
            return (result);
        } else {
            //一个区块也无法提供
            size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
            //试着让残余内存还有利用价值
            if (bytes_left > 0) {
                block *volatile *my_free_list = free_list + freelist_index(bytes_left);
                ((block *) start_free)->free_list_link = *my_free_list;
                *my_free_list = (block *) start_free;
            }
            // 配置heap空间，用来补充内存池
            start_free = (char *) malloc(bytes_to_get);
            if (0 == start_free) {
                // 空间不足
                int i;
                block *volatile *my_free_list, *p;
                //试着检视我们手上拥有的东西，搜寻适当的free_list
                for (i = size; i <= _max_bytes; i += _align) {
                    my_free_list = free_list + freelist_index(i);
                    p = *my_free_list;
                    if (0 != p) { //free_list中尚有未用区块
                        //调整free_list以释放未用区块
                        *my_free_list = p->free_list_link;
                        start_free = (char *) p;
                        end_free = start_free + i;
                        //递归调用自己，修正nobjs
                        return (chunk_alloc(size, nobjs));
                        //任何残余碎片终将被编入合适的free_list中备用
                    }
                }
                end_free = 0;//无可用内存
                //调用第一级配置器，看看oom机制能否帮忙
                start_free = (char *) malloc_alloc::allocate(bytes_to_get);

            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            //递归调用自己，修正nobjs
            return (chunk_alloc(size, nobjs));
        }
    }
}
#endif //MY_STL_ALLOC_H
