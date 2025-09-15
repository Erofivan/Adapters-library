#pragma once

#include <memory>
#include <string>

static std::string _dummy_loud_used;
static std::string* _loud_used_ptr = &_dummy_loud_used;

inline void set_loud_target(std::string& s){ _loud_used_ptr = &s; }
inline void reset_loud_target(){ _loud_used_ptr = &_dummy_loud_used; }

template <class T>
struct LoudAlloc: public std::allocator<T> {

    using std::allocator<T>::allocator;

    bool used = false;

    T* allocate( std::size_t n ){
        *_loud_used_ptr = "used";
        return std::allocator<T>::allocate(n);
    }
};