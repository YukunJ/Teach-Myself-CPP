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
