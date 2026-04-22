#ifndef TEXTEDITOR_VECTOR_H
#define TEXTEDITOR_VECTOR_H

#include <stddef.h>
#include <new>

namespace TextEditor {

template <typename T>
class Vector {
public:
    Vector()
        : size_(0),
          capacity_(0),
          items_(NULL) {
    }

    Vector(const Vector &other)
        : size_(0),
          capacity_(0),
          items_(NULL) {
        CopyFrom(other);
    }

    ~Vector() {
        DestroyRange(0, size_);
        ::operator delete(items_);
    }

    Vector &operator=(const Vector &other) {
        if (this != &other) {
            Vector copy(other);
            swap(copy);
        }
        return *this;
    }

    void swap(Vector &other) {
        const size_t size = size_;
        const size_t capacity = capacity_;
        T *items = items_;

        size_ = other.size_;
        capacity_ = other.capacity_;
        items_ = other.items_;

        other.size_ = size;
        other.capacity_ = capacity;
        other.items_ = items;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    T *data() {
        return items_;
    }

    const T *data() const {
        return items_;
    }

    T &operator[](size_t index) {
        return items_[index];
    }

    const T &operator[](size_t index) const {
        return items_[index];
    }

    void reserve(size_t requested_capacity) {
        EnsureCapacity(requested_capacity);
    }

    void push_back(const T &value) {
        EnsureCapacity(size_ + 1);
        new (items_ + size_) T(value);
        ++size_;
    }

    int find(const T &value) const {
        for (size_t i = 0; i < size_; ++i) {
            if (items_[i] == value) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    void replace(const T &existing_value, const T &replacement) {
        const int index = find(existing_value);
        if (index >= 0) {
            items_[index] = replacement;
        }
    }

    void pop_back() {
        if (!size_) {
            return;
        }

        --size_;
        items_[size_].~T();
    }

    void remove_at(size_t index) {
        if (index >= size_) {
            return;
        }

        for (size_t i = index + 1; i < size_; ++i) {
            items_[i - 1] = items_[i];
        }

        pop_back();
    }

    void clear() {
        DestroyRange(0, size_);
        size_ = 0;
    }

private:
    void CopyFrom(const Vector &other) {
        if (!other.size_) {
            return;
        }

        EnsureCapacity(other.size_);
        for (size_t i = 0; i < other.size_; ++i) {
            new (items_ + i) T(other.items_[i]);
        }
        size_ = other.size_;
    }

    void DestroyRange(size_t start, size_t end) {
        for (size_t i = end; i > start; --i) {
            items_[i - 1].~T();
        }
    }

    void EnsureCapacity(size_t requested_capacity) {
        if (requested_capacity <= capacity_) {
            return;
        }

        size_t new_capacity = capacity_ ? capacity_ * 2 : 4;
        if (new_capacity < requested_capacity) {
            new_capacity = requested_capacity;
        }

        T *new_items = static_cast<T *>(::operator new(sizeof(T) * new_capacity));
        for (size_t i = 0; i < size_; ++i) {
            new (new_items + i) T(items_[i]);
        }

        DestroyRange(0, size_);
        ::operator delete(items_);
        items_ = new_items;
        capacity_ = new_capacity;
    }

    size_t size_;
    size_t capacity_;
    T *items_;
};

}  // namespace TextEditor

#endif
