#include <iostream>

enum class shape_type {
    circle,
    triangle,
    rectangle
};

class shape {
public:
    shape() { std::cout << "shape ctor" << std::endl; }
    virtual ~shape() { std::cout << "shape dtor" << std::endl; }
    virtual void print() { std::cout << "I'm a shape" << std::endl; }
};

class circle: public shape {
public:
    circle() { std::cout << "circle ctor" << std::endl; }
    virtual ~circle() { std::cout << "circle dtor" << std::endl; }
    virtual void print() override { std::cout << "I'm a circle" << std::endl; }
};

class triangle: public shape {
public:
    triangle() { std::cout << "triangle ctor" << std::endl; }
    virtual ~triangle() { std::cout << "triangle dtor" << std::endl; }
    virtual void print() override { std::cout << "I'm a triangle" << std::endl; }
};

class rectangle: public shape {
public:
    rectangle() { std::cout << "rectangle ctor" << std::endl; }
    virtual ~rectangle() { std::cout << "rectangle dtor" << std::endl; }
    virtual void print() override { std::cout << "I'm a rectangle" << std::endl; }
};

shape *create_shape(shape_type type) {
    /* factory method for shape hierarchy */
    switch (type) {
        case shape_type::circle:
            return new circle();
        case shape_type::triangle:
            return new triangle();
        case shape_type::rectangle:
            return new rectangle();
    }
}
