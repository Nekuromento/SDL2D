#ifndef defines_h__
#define defines_h__

#if !defined(__GNUC__) && !defined(__clang__)
#  pragma warning(disable: 4514)
#  pragma warning(disable: 4141)

#  define REALLY_INLINE __forceinline
#  define NOEXCEPT throw()
#else
#  define REALLY_INLINE __attribute__((always_inline))
#  define NOEXCEPT noexcept
#endif

#endif // defines_h__
