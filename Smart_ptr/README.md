### How to write your own smart pointers?

Unarguably, one of the most important changes in modern C++ is the introduction of smart pointers. It essentially help programmer to manage dynamically-allocated space and reduce the usage of raw `new` and `delete` maximally.

In this section, we will write a basic version of our own smart pointers: **unique_ptr** and **shared_ptr**. This is not only good exercise to deepen understanding of them, but also they appear frequently as C++ interview questions. (I got asked this morning when interviewing with [Tower](https://www.tower-research.com): "How would you design a unique_ptr yourself?") 

My reference in this section could be found [here](https://github.com/Light-City/CPlusPlusThings/tree/master/learn_class/modern_C++_30/smart_ptr).

---

#### unique_ptr

**std::unique_ptr** is used to manage exclusive ownership of an object's lifetime. It does not allow copy, but it is movable. Upon out of scope, the **std::unique_ptr** will properly release the space allocated for the pointer it is managing. And we may transfer the exclusive ownership of the object using move semantics.

Key implementation points are commented in the code below. The [source file](./unique_ptr.cpp) also contains test suites for illustration. 
```CPP
namespace yukunj {

template<typename T>
class unique_ptr {
public:
    explicit unique_ptr(T *ptr = nullptr) noexcept
    : ptr_(ptr) {}
    
    /* disable copy ability to ensure exclusive ownership */
    unique_ptr(const unique_ptr& rhs) = delete;
    unique_ptr &operator=(const unique_ptr& rhs) = delete;
    
    /* enable move operations for ownership transfer */
    unique_ptr(unique_ptr&& rhs) noexcept {
        ptr_ = rhs.release();
    }
    unique_ptr &operator=(unique_ptr&& rhs) noexcept {
        ptr_ = rhs.release();
        return *this;
    }
    
    /* upon destruction, delete the managed ptr */
    ~unique_ptr() {
        if (ptr_) {
            delete ptr_;
        }
    }
    
    /* mimic raw pointer's deference operator '*' */
    T &operator*() const noexcept { return *ptr_; }
    
    /* mimic raw pointer's access operator '->' */
    T *operator->() const noexcept { return ptr_; }
    
    /* nullptr test */
    operator bool() const noexcept { return ptr_; }
    
    /* give access to the managed ptr */
    T *get() const noexcept { return ptr_; }
    
    /* release the ownership of the managed ptr */
    T *release() noexcept {
        T *tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }
    
    /* helper function to grab other unique_ptr's ptr */
    void swap(unique_ptr&& rhs) noexcept {
        using std::swap;
        swap(ptr_, rhs.ptr_);
    }
private:
    T *ptr_;
};

} // namespace yukunj
```

---

#### shared_ptr

**unique_ptr** is great when we want to ensure the exclusive ownership of an object. However, in some case we want that, multiple pointers might point to the same object. And when the last referencing pointer goes out of scope, we want to release the dynamically-allocated space. 

This is the **shared_ptr** we will introduce here. Multiple **shared_ptr** will share not only the same pointer to object, but also a **counter**. When the **counter** goes down to 0 (no one pointing to the object anymore), we need to delete both the object and the **counter** all together.

First, we implement a simple **counter** class as follows:

```CPP
/* light-weight counter taking up 8 bytes */
class counter {
public:
    counter() : count_(1) {}

    void add_count() { count_++; }

    size_t reduce_count() { return --count_; }

    size_t get_count() { return count_; }

private:
    size_t count_;
};
```

Built upon this **counter** class, we create our version of **shared_ptr**. It enables both copy and move operations. In copy operations the counter will be incremented by 1, while the move operation is just "steal over" from the rvalue referenced **shared_ptr**.

Key implementation points are commented in the code below. The [source file](./shared_ptr.cpp) also contains test suites for illustration.

```CPP
namespace yukunj {

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
```
