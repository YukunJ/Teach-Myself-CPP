#include <iostream>
#include <assert.h>
#include "shape.h"

namespace yukunj {

 /*
  simple counter class to keep record of
  how many pointers are pointing to a shared object
 */
class counter {
public:
    counter() : count_(1) {}

    void add_count() { count_++; }

    size_t reduce_count() { return --count_; }

    size_t get_count() { return count_; }

private:
    size_t count_;
};

template<typename T>
class shared_ptr {
public:
    explicit shared_ptr(T *ptr = nullptr) noexcept : ptr_(ptr) {
        if (ptr) {
            counter_ = new counter();
        }
    }

    ~shared_ptr() {
        /*
         when the last referencing pointer is deleted
         delete the object pointer and counter together
        */
        if (ptr_ && !counter_->reduce_count()) {
            delete ptr_;
            delete counter_;
        }
    }

    shared_ptr(const shared_ptr& rhs) {
        /*
         copy ctor, increase counter by 1 and copy over
         both object ptr and counter ptr
         */
        if (rhs.ptr_) {
            rhs.counter_->add_count();
            ptr_ = rhs.get();
            counter_ = rhs.get_counter();
        }
    }

    shared_ptr(shared_ptr&& rhs) {
        /*
         move ctor, steal over the counter and object ptr
        */
        if (rhs.ptr_) {
            ptr_ = rhs.get();
            counter_ = rhs.get_counter();
            rhs.ptr_ = nullptr;
            rhs.counter_ = nullptr;
        }
    }

    shared_ptr &operator=(const shared_ptr& rhs) {
        /*
         copy assignment, increase counter by 1 and copy over
        */
        if (rhs.ptr_) {
            rhs.counter_->add_count();
            ptr_ = rhs.get();
            counter_ = rhs.get_counter();
        }
        return *this;
    }

    shared_ptr &operator=(shared_ptr&& rhs) {
        /*
         move assignment, steal over from right hand side
        */
        if (rhs.ptr_) {
            ptr_ = rhs.get();
            counter_ = rhs.get_counter();
            rhs.ptr_ = nullptr;
            rhs.counter_ = nullptr;
        }
        return *this;
    }

    T &operator*() const noexcept { return *ptr_; }

    T *operator->() const noexcept { return ptr_; }

    operator bool() const noexcept { return ptr_; }

    T *get() const noexcept { return ptr_; }

    counter *get_counter() const noexcept { return counter_; }

    size_t use_count() const noexcept {
        /* how many pointers are pointing at this object */
        return (ptr_) ? counter_->get_count() : 0;
    }

private:
    T *ptr_;
    counter *counter_;
};

} // namespace yukunj

int main(int argc, const char *argv[]) {
    // copying over the shared_ptr will increase the count
    yukunj::shared_ptr<shape> ptr1{create_shape(shape_type::triangle)};
    yukunj::shared_ptr<shape> ptr2 = ptr1; // copy assignment
    yukunj::shared_ptr<shape> ptr3{ptr2}; // copy ctor
    assert(ptr3.use_count() == 3);

    // move over the shared_ptr wouldn't increase the count
    yukunj::shared_ptr<shape> ptr4{std::move(ptr3)};
    assert(!ptr3);
    assert(ptr4.use_count() == 3);

    // upon out of scope, will reduce the count
    {
        yukunj::shared_ptr<shape> in_scope = ptr1;
        assert(ptr1.use_count() == 4);
    }
    assert(ptr1.use_count() == 3);

    return 0;
}
