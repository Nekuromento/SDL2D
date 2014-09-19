#ifndef String_h__
#define String_h__

#include <cstdint>
#include <iterator>
#include <algorithm>

class String {
    struct Header {
        const uint16_t length;
        uint16_t refCount;

        Header(const uint16_t size) :
            length {size},
            refCount {1}
        {}
    };

    const char* _data;

    static char* allocate(const size_t size);

    Header* header() const;

    void incRef();
    void decRef();

    enum InitPolicy {
        Copy,
        NoCopy
    };

    String& init(const char* const string, const size_t length, const InitPolicy policy);

    String concatenate(const char* const string, const size_t length) const;

public:
    typedef const char* iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    // By default overload resolution always uses 'const char*' constructor
    // even for static arrays and literals, so we fix that with a wrapper
    // with implicit constructor
    struct ConstChar {
        const char* const string;

        ConstChar(const char* const string) :
            string {string}
        {}
    };

    String();
    String(decltype(nullptr) nil);
    String(const ConstChar& string);
    String(const ConstChar& string, const size_t length);

    template <size_t N>
    String(const char (&string)[N], const size_t length = N - 1) {
        init(string, length, Copy);
    }

    String(const String& other);
    String(String&& other);
    ~String();

    String& operator =(const String& other);
    String& operator =(String&& other);
    String& operator =(const ConstChar& string);

    template <size_t N>
    String& operator =(const char (&string)[N]) {
        String(string, N - 1).swap(*this);
        return *this;
    }

    void swap(String& other);

    bool operator <(const String& other) const;
    bool operator <(const ConstChar& string) const;

    template <size_t N>
    bool operator <(const char (&string)[N]) const {
        return std::lexicographical_compare(begin(), end(), string, string + N - 1);
    }

    bool operator ==(const String& other) const;
    bool operator !=(const String& other) const;

    bool operator ==(const ConstChar& string) const;
    bool operator !=(const ConstChar& string) const;

    template <size_t N>
    bool operator ==(const char (&string)[N]) const {
        return size() == N -1 && std::equal(begin(), end(), string);
    }

    template <size_t N>
    bool operator !=(const char (&string)[N]) const {
        return !(operator==(string));
    }

    size_t size() const;
    bool empty() const;

    iterator begin() const;
    iterator end() const;
    reverse_iterator rbegin() const;
    reverse_iterator rend() const;

    String operator +(const String& other) const;
    String operator +(const ConstChar& string) const;

    template <size_t N>
    String operator +(const char (&string)[N]) const {
        if (!_data)
            return String(string, N - 1);
        return concatenate(string, N - 1);
    }

    String& operator +=(const String& other);
    String& operator +=(const ConstChar& string);

    template <size_t N>
    String& operator +=(const char (&string)[N]) const {
        if (!_data)
            return operator=(string);
        return operator=(concatenate(string.string, N - 1));
    }
};

#endif // String_h__
