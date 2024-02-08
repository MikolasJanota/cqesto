/*
 * File:  immutable_vector.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:47:20 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
namespace qesto {
template <class T, class HashFun> class const_ImmutableVectorIterator;
template <class T, class HashFun> class ImmutableVector {
  public:
    typedef const_ImmutableVectorIterator<T, HashFun> const_iterator;

  public:
    inline ImmutableVector() {
        _data = nullptr;
        _size = 0;
        _hash_code = EMPTY_HASH;
    }

    ImmutableVector(const std::vector<T> &es);
    virtual ~ImmutableVector() { decrease(); }

    inline ImmutableVector(const ImmutableVector<T, HashFun> &ls) {
        _hash_code = ls._hash_code;
        _size = ls._size;
        _data = ls._data;
        if (_data != nullptr)
            ++(_data->count);
    }

    inline ImmutableVector(ImmutableVector<T, HashFun> &&o)
        : _hash_code(std::exchange(o._hash_code, -1)),
          _size(std::exchange(o._size, -1)),
          _data(std::exchange(o._data, nullptr)) {}

    ImmutableVector<T, HashFun> &
    operator=(const ImmutableVector<T, HashFun> &ls) {
        decrease();
        _hash_code = ls._hash_code;
        _size = ls._size;
        _data = ls._data;
        if (_data != nullptr)
            ++(_data->count);
        return *this;
    }

    bool equals(const ImmutableVector<T, HashFun> &other) const;
    std::ostream &print(std::ostream &out) const;
    inline size_t hash_code() const { return _hash_code; }
    inline size_t size() const { return _size; }
    inline bool empty() const { return _size == 0; }
    inline const_iterator begin() const;
    inline const_iterator end() const;
    inline const T operator[](size_t index) const {
        assert(index < _size);
        return _data->elements[index];
    }
    static ImmutableVector<T, HashFun>
    conc(const ImmutableVector<T, HashFun> &a,
         const ImmutableVector<T, HashFun> &b) {
        const auto asz = a.size();
        const auto bsz = b.size();
        if (!asz)
            return b;
        if (!bsz)
            return a;
        std::vector<T> rv(asz + bsz);
        for (size_t i = 0; i < asz; ++i)
            rv[i] = a[i];
        for (size_t i = 0; i < bsz; ++i)
            rv[asz + i] = b[i];
        return ImmutableVector<T, HashFun>(rv);
    }

  private:
    struct Data {
        size_t count;
        T *elements;
    };
    static const size_t EMPTY_HASH = 3217;
    size_t _hash_code;
    size_t _size;
    Data *_data;
    inline void decrease(); // decrease reference counter
};

template <class T, class HashFun>
inline void ImmutableVector<T, HashFun>::decrease() {
    if (_data == nullptr)
        return;
    assert(_data->count);
    --(_data->count);
    if ((_data->count) == 0) {
        delete[] _data->elements;
        delete _data;
    }
    _data = nullptr;
}

template <class T, class HashFun>
bool ImmutableVector<T, HashFun>::equals(
    const ImmutableVector<T, HashFun> &other) const {
    if (other._size != _size)
        return false;
    if (other._data == _data)
        return true;

    for (size_t i = _size; i--;)
        if (_data->elements[i] != other._data->elements[i])
            return false;
    return true;
}

template <class T, class HashFun> class const_ImmutableVectorIterator {
  public:
    const_ImmutableVectorIterator(const ImmutableVector<T, HashFun> &ls,
                                  size_t x)
        : ls(ls), i(x) {}
    const_ImmutableVectorIterator(
        const const_ImmutableVectorIterator<T, HashFun> &mit)
        : ls(mit.ls), i(mit.i) {}
    const_ImmutableVectorIterator &operator++() {
        ++i;
        return *this;
    }
    bool operator==(const const_ImmutableVectorIterator<T, HashFun> &rhs) {
        assert(&ls == &(rhs.ls));
        return i == rhs.i;
    }
    bool operator!=(const const_ImmutableVectorIterator<T, HashFun> &rhs) {
        assert(&ls == &(rhs.ls));
        return i != rhs.i;
    }
    const T operator*() const { return ls[i]; }

  private:
    const ImmutableVector<T, HashFun> &ls;
    size_t i;
};

template <class T, class HashFun>
inline typename ImmutableVector<T, HashFun>::const_iterator
ImmutableVector<T, HashFun>::begin() const {
    return const_ImmutableVectorIterator<T, HashFun>(*this, 0);
}

template <class T, class HashFun>
inline typename ImmutableVector<T, HashFun>::const_iterator
ImmutableVector<T, HashFun>::end() const {
    return const_ImmutableVectorIterator<T, HashFun>(*this, _size);
}

template <class T, class HashFun>
ImmutableVector<T, HashFun>::ImmutableVector(const std::vector<T> &es) {
    const size_t vsz = es.size();
    if (vsz == 0) {
        _data = nullptr;
        _size = 0;
        _hash_code = EMPTY_HASH;
        return;
    }
    _data = new Data();
    _data->elements = new T[vsz];
    _data->count = 1;
    T *const elements = _data->elements;
    for (size_t i = 0; i < vsz; ++i)
        elements[i] = es[i];
    std::sort(elements, elements + vsz);
    _size = vsz;
    size_t j = 1;
    T last = elements[0];
    for (size_t i = 1; i < vsz; ++i) {
        if (elements[i] == last)
            continue;
        elements[j] = elements[i];
        last = elements[i];
        ++j;
    }
    _size = j;
    _hash_code = 7;
    for (size_t i = 0; i < _size; ++i)
        _hash_code = _hash_code * 31 + HashFun()(elements[i]);
}

template <class T, class HashFun> class ImmutableVector_equal {
  public:
    inline bool operator()(const ImmutableVector<T, HashFun> &v1,
                           const ImmutableVector<T, HashFun> &v2) const {
        return v1.equals(v2);
    }
};

template <class T, class HashFun> struct ImmutableVector_hash {
    inline size_t operator()(const ImmutableVector<T, HashFun> &ls) const {
        return ls.hash_code();
    }
};

template <class T, class HashFun>
std::ostream &ImmutableVector<T, HashFun>::print(std::ostream &o) const {
    const size_t sz = size();
    o << "[";
    for (size_t i = 0; i < sz; ++i) {
        if (i)
            o << " ";
        o << (*this)[i];
    }
    return o << "]";
}

template <class T, class HashFun>
std::ostream &print(std::ostream &o, const ImmutableVector<T, HashFun> &vs) {
    return vs.print(o);
}

template <class T, class HashFun>
ImmutableVector<T, HashFun> join(ImmutableVector<T, HashFun> a,
                                 ImmutableVector<T, HashFun> b) {
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    std::vector<T> d(a.size() + b.size());
    for (size_t i = 0; i < a.size(); ++i)
        d[i] = a[i];
    size_t o = a.size();
    for (size_t i = 0; i < b.size(); ++i, ++o)
        d[o] = b[i];
    return ImmutableVector<T, HashFun>(d);
}

template <class T, class HashFun>
bool contains(ImmutableVector<T, HashFun> es, const T &e) {
    for (const auto &i : es)
        if (i == e)
            return true;
    return false;
}
} // namespace qesto
