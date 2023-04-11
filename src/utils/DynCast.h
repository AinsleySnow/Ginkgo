#ifndef _DYN_CAST_H_
#define _DYN_CAST_H_

#define ENABLE_IS               \
template<class T>               \
bool Is() const                 \
{                               \
    return T::ClassOf(this);    \
}

#define ENABLE_AS                           \
template<class T>                           \
T* As()                                     \
{                                           \
    if (!Is<T>()) return nullptr;           \
    return static_cast<T*>(this);           \
}                                           \
                                            \
template<class T>                           \
const T* As() const                         \
{                                           \
    if (!Is<T>()) return nullptr;           \
    return static_cast<const T*>(this);     \
}                                           \

#endif // _DYN_CAST_H_
