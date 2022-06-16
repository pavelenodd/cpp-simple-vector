#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>

#include "array_ptr.h"

struct CapacityReserve {
    CapacityReserve (size_t capacity)
    :capacity_(capacity)
    {}
    
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value = 0)
    : capacity_(size), size_(size), elements_(size)
    {
        std::fill(begin(), end(), value);
    }
    
    SimpleVector(size_t size, Type&& value)
    : capacity_(size), size_(size), elements_(size)
    {
        std::fill(begin(), end(), value);
    }
    
    SimpleVector(CapacityReserve res) 
    : capacity_(res.capacity_)    {
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(const std::initializer_list<Type>& init) 
    : capacity_(init.size()), size_(init.size()), elements_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }
    SimpleVector(std::initializer_list<Type>&& init) 
    : capacity_(init.size()), size_(init.size()), elements_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }
    

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index <= size_);
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index <= size_);
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size");
        }
        return elements_[index];
    }
    const Type& At(size_t index) const {
        if (index >= size_) {
            try {throw std::out_of_range("index >= size");}
            catch (std::out_of_range&) {throw;}
        }
        return elements_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t x = new_size;
            if (capacity_ * 2 > x) {
                x = capacity_ * 2;
            }
            ArrayPtr<Type> tmp(x);            
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), tmp.Get());            
            elements_.swap(tmp);
            for (auto first = begin() + size_; first != begin() + new_size -1; ++first) {
                Type a = Type();
                *first = std::exchange(a, Type());
            }            
            capacity_ = x;
        } else if (new_size > size_) {
            for (auto first = begin() + size_; first != begin() + new_size -1; ++first) {
                Type a = Type();
                *first = std::exchange(a, Type());
            }  
        }
        size_ = new_size;
    }

    
    Iterator begin() noexcept {
        return elements_.Get();
    }
    Iterator end() noexcept {
        return begin() + size_;
    }
    ConstIterator begin() const noexcept {
        return elements_.Get();
    }
    ConstIterator end() const noexcept {
        return begin() + size_;
    }
    ConstIterator cbegin() const noexcept {
        return elements_.Get();
    }
    ConstIterator cend() const noexcept {
        return begin() + size_;
    }
    
    
     SimpleVector(const SimpleVector& other) {
        Resize(other.GetSize());
        std::copy(other.begin(), other.end(), begin());
    }
    SimpleVector(SimpleVector&& other) {
        Resize(other.GetSize());
        std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());
        other.size_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }
    SimpleVector& operator=(SimpleVector&& rhs) {
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Resize(size_ + 1);
        elements_[size_ - 1] = item;
    }
    void PushBack(Type&& item) {
        Resize(size_ + 1);
        elements_[size_ - 1] = std::exchange(item, 0);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert (pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        Resize(size_ + 1);
        std::copy_backward(begin() + dist, end() - 1, end());
        elements_[dist] = std::exchange(value, 0);
        return &elements_[dist];
    }
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert (pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        Resize(size_ + 1);
        std::copy_backward(std::make_move_iterator(begin() + dist), std::make_move_iterator(end() - 1), end());
        elements_[dist] = std::exchange(value, 0);
        return &elements_[dist];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert (!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert (!IsEmpty());
        assert (pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        std::copy(std::make_move_iterator(begin() + dist + 1), std::make_move_iterator(end()), begin() + dist);
        --size_;
        return begin() + dist;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        elements_.swap(other.elements_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(begin(), end(), tmp.Get());
            elements_.swap(tmp);
            capacity_ = new_capacity;
        }
    }
    
    
private:    
    size_t capacity_ = 0;
    size_t size_ = 0;
    ArrayPtr<Type> elements_;
    
};



CapacityReserve Reserve(size_t capacity) {
    return CapacityReserve(capacity);
}


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;   
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs > rhs || lhs == rhs;
}