#ifndef Event_h__
#define Event_h__

#include <algorithm>
#include <vector>

#include "Delegate.hpp"
#include "Util/CapacityArray.hpp"
#include "Util/noncopyable.hpp"

template <typename T>
class Event {
    static_assert(util::False<T>::value, "Only callable arguments are supported");
};

template <typename ReturnType, typename... Args>
class Event<ReturnType (Args...)> : public util::Noncopyable {
    typedef Delegate<ReturnType (Args...)> DelegateType;
    typedef util::CapacityArray<DelegateType> DelegateList;

    DelegateList _handlers;
    mutable bool* _deletedInInvoke;

public:
    template <typename Allocator>
    explicit Event(Allocator& alloc, const size_t capacity = 1) :
        _handlers{ alloc, capacity },
        _deletedInInvoke{ nullptr }
    {}

    template <typename Allocator>
    Event(Allocator& alloc, Event& other) :
        _handlers{ alloc, other._handlers },
        _deletedInInvoke{ nullptr }
    {}

    Event(Event&& other) :
        _handlers{ std::move(other._handlers) },
        _deletedInInvoke{ other._deletedInInvoke }
    {
        other._handlers = DelegateList();
        other._deletedInInvoke = nullptr;
    }

    ~Event() {
        // signal invoke function that we are deleted
        if (_deletedInInvoke) {
            *_deletedInInvoke = true;
        }
    }

    Event& operator =(Event&& other) NOEXCEPT {
        other.swap(*this);

        return *this;
    }

    void swap(Event& other) NOEXCEPT {
        _handlers.swap(other._handlers);
        std::swap(_deletedInInvoke, other._deletedInInvoke);
    }

    bool empty() const NOEXCEPT {
        return _handlers.empty();
    }

    void subscribe(DelegateType handler) {
        assert(("Trying to subscribe an empty handler", handler));
        auto pos = std::find(_handlers.begin(), _handlers.end(), handler);
        if (pos == _handlers.end())
            _handlers.add(std::move(handler));
    }

    void unsubscribe(const DelegateType& handler) {
        _handlers.remove(handler);
    }

    void clear() {
        _handlers.clear();
    }

    //TODO: use allocator for copying vector
    template <typename Allocator>
    void invoke(Allocator& alloc, Args&& ...args) const {
        assert(("Invoking event in the middle of another invocation", !_deletedInInvoke));

        bool deleted = false;
        _deletedInInvoke = &deleted;

        for (const auto& handler : DelegateList(alloc, _handlers)) {
            if (deleted)
                break;

            handler.invoke(std::forward<Args>(args)...);
        }

        if (!deleted)
            _deletedInInvoke = nullptr;
    }
};

#endif // Event_h__
