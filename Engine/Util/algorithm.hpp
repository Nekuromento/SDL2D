#ifndef algorithm_h__
#define algorithm_h__

#include <algorithm>

namespace util {

template <typename T>
T clamp(const T& value, const T& lo, const T& hi) {
    return std::min(hi, std::max(lo, value));
}

template <typename T, typename Predicate>
T clamp(const T& value, const T& lo, const T& hi, Predicate p) {
    return std::min(hi, std::max(lo, value, p), p);
}

template <typename BidirectionalIterator, typename Predicate>
std::pair<BidirectionalIterator, BidirectionalIterator> gather(BidirectionalIterator first,
                                                               BidirectionalIterator last,
                                                               BidirectionalIterator pivot,
                                                               Predicate p) {
    return std::make_pair(std::stable_partition(first, pivot, std::not1(p)),
                          std::stable_partition(pivot, last, p));
}

}

#endif // algorithm_h__