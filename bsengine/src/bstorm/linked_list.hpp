#pragma once

#include <bstorm/nullable_shared_ptr.hpp>

#include <cassert>

namespace bstorm
{
template <class T>
class LinkedListIterator;
template <class T>
class LinkedList
{
public:
    LinkedList() :
        head_(nullptr),
        tail_(nullptr)
    {
    }

    static const std::shared_ptr<LinkedList<T>>& Empty()
    {
        static auto empty = std::make_shared<LinkedList<T>>();
        return empty;
    }

    LinkedList(const std::shared_ptr<T>& elem, const std::shared_ptr<LinkedList<T>>& tail) :
        head_(elem),
        tail_(tail)
    {
        if (head_ == nullptr)
        {
            assert(head_ != nullptr);
        }
        assert(tail_ != nullptr);
    }

    static std::shared_ptr<LinkedList<T>> Cons(const std::shared_ptr<T>& elem, const std::shared_ptr<LinkedList<T>>& tail)
    {
        return std::make_shared<LinkedList<T>>(elem, tail);
    }

    const std::shared_ptr<T>& GetHead() const
    {
        assert(!IsEmpty());
        return head_;
    }
    const std::shared_ptr<LinkedList<T>>& GetTail() const
    {
        assert(!IsEmpty());
        return tail_;
    }

    LinkedListIterator<T> begin() const
    {
        return LinkedListIterator<T>(head_, tail_);
    }

    LinkedListIterator<T> end() const
    {
        return LinkedListIterator<T>();
    }

    bool IsEmpty() const { return head_ == nullptr && tail_ == nullptr; }
private:
    std::shared_ptr<T> head_;
    std::shared_ptr<LinkedList<T>> tail_;
};


template <class T>
class LinkedListIterator
{
public:
    LinkedListIterator(const std::shared_ptr<T>& head, const std::shared_ptr<LinkedList<T>>& tail) :
        head_(head),
        tail_(tail)
    {
    }

    // end
    LinkedListIterator() :
        head_(nullptr),
        tail_(nullptr)
    {
    }

    T& operator *()
    {
        assert(head_ != nullptr);
        return *head_;
    }

    void operator ++()
    {
        assert(tail_ != nullptr);
        head_ = tail_->GetHead();
        tail_ = tail_->GetTail();
    }

    bool operator !=(LinkedListIterator<T>& it)
    {
        return !(head_ == nullptr && tail_ == nullptr && it.head_ == nullptr && it.tail_ == nullptr);
    }

private:
    std::shared_ptr<T> head_;
    std::shared_ptr<LinkedList<T>> tail_;
};
}