#include "String.hpp"

#include <cassert>

#include "Memory/SmallObjectPool.hpp"

char* String::allocate(const size_t size) {
    const size_t headerSize = sizeof(Header);
    // request enough size for string, null terminator and header
    const size_t allocSize = size + 1 + headerSize;

    void * memory =
        SmallObjectPool::getDefault().allocate(allocSize, 1, headerSize);
    new (memory) Header(size);

    return static_cast<char*>(memory) + headerSize;
}

String::Header* String::header() const {
    return reinterpret_cast<Header*>(const_cast<char*>(_data)) - 1;
}

void String::incRef() {
    if (!_data)
        return;

    ++header()->refCount;
}

void String::decRef() {
    if (!_data)
        return;

    --header()->refCount;

    if (!header()->refCount)
        SmallObjectPool::getDefault().free(header());

    _data = nullptr;
}

String& String::init(const char* const string, const size_t length, const InitPolicy policy) {
    if (!string) {
        _data = nullptr;
        return *this;
    }

    if (policy == Copy) {
        char* buffer = allocate(length);
        memcpy(buffer, string, length);
        buffer[length] = '\0';
        _data = buffer;
    } else {
        _data = string;
    }

    assert(header()->length == length);
    assert(header()->refCount == 1);
    return *this;
}

String String::concatenate(const char* const string, const size_t length) const {
    const size_t oldLength = header()->length;
    const size_t newLength = oldLength + length;

    char* buffer = allocate(newLength);

    memcpy(buffer, _data, oldLength);
    memcpy(buffer + oldLength, string, length);

    buffer[newLength] = '\0';

    return String().init(buffer, newLength, NoCopy);
}

String::String() :
    _data {nullptr}
{}

String::String(decltype(nullptr) nil) :
    _data {nullptr}
{}

String::String(const ConstChar& string) {
    init(string.string, strlen(string.string), Copy);
}

String::String(const ConstChar& string, const size_t length) {
    init(string.string, length, Copy);
}

String::String(const String& other) :
    _data(other._data)
{
    incRef();
}

String::String(String&& other) :
    _data(other._data)
{
    other._data = nullptr;
}

String::~String() {
    decRef();
}

String& String::operator=(const String& other) {
    String(other).swap(*this);

    return *this;
}

String& String::operator=(String&& other) {
    other.swap(*this);

    return *this;
}

String& String::operator=(const ConstChar& string) {
    String(string.string).swap(*this);

    return *this;
}

void String::swap(String& other) {
    std::swap(_data, other._data);
}

bool String::operator<(const String& other) const {
    return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
}

bool String::operator<(const ConstChar& string) const {
    return std::lexicographical_compare(begin(), end(), string.string, string.string + strlen(string.string));
}

bool String::operator==(const String& other) const {
    return size() == other.size() && std::equal(begin(), end(), other.begin());
}

bool String::operator==(const ConstChar& string) const {
    return size() == strlen(string.string) && std::equal(begin(), end(), string.string);
}

bool String::operator!=(const String& other) const {
    return !(operator==(other));
}

bool String::operator!=(const ConstChar& string) const {
    return !(operator==(string));
}

size_t String::size() const {
    return _data ? header()->length : 0;
}

bool String::empty() const {
    return size() == 0;
}

String::iterator String::begin() const {
    return _data;
}

String::iterator String::end() const {
    return _data + size();
}

String::reverse_iterator String::rbegin() const {
    return reverse_iterator(end());
}

String::reverse_iterator String::rend() const {
    return reverse_iterator(begin());
}

String String::operator+(const String& other) const {
    if (!_data)
        return other;

    return concatenate(other._data, other.header()->length);
}

String String::operator+(const ConstChar& string) const {
    if (!_data)
        return String(string);

    return concatenate(string.string, strlen(string.string));
}

String& String::operator+=(const String& other) {
    if (!_data)
        return operator=(other);

    return operator=(concatenate(other._data, other.header()->length));
}

String& String::operator+=(const ConstChar& string) {
    if (!_data)
        return operator=(string);

    return operator=(concatenate(string.string, strlen(string.string)));
}
