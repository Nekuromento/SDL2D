#ifndef noncopyable_h__
#define noncopyable_h__

namespace util {

class Noncopyable {
protected:
#ifdef __GNUC__
    Noncopyable() = default;
    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
#else
    Noncopyable() {}
    ~Noncopyable() {}

private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);
#endif
};

}
#endif // noncopyable_h__