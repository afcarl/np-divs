#ifndef UTILS_HPP_
#define UTILS_HPP_
#include "basics.hpp"

#include <algorithm>
#include <vector>

#include <boost/math/special_functions.hpp>

// code in here because templates are silly :/

template <typename T>
bool cmp_with_inf(T i, T j) { return boost::math::isinf(i) || i < j; }

template <typename T>
struct greater_than {
    greater_than(T x) : base(x) {}
    bool operator()(T y) { return y > base; }

    private:
        T base;
};

template <typename T>
void fix_terms(std::vector<T> &terms, double ub = .99) {
    /* Takes a vector of elements and replaces any infinite or very-large
     * elements with the value of the highest non-very-large element, as well
     * as possibly changing the order. "Very-large" is defined as the ub-th
     * quantile if ub < 1, otherwise the largest non-inf element.
     */
    typedef typename std::vector<T>::size_type sz;

    using std::max_element;
    using std::nth_element;
    using std::replace_if;

    T cutoff;
    bool find_noninf_max = true;

    // try finding the ub-th percentile
    if (ub < 1) {
        sz k = terms.size() * ub; // the index we want
        nth_element(terms.begin(), terms.begin() + k, terms.end());
        cutoff = terms[k];

        find_noninf_max = boost::math::isinf(cutoff);
    }

    // just use the highest non-inf element
    if (find_noninf_max) {
        cutoff = *max_element(terms.begin(), terms.end(), cmp_with_inf<T>);
    }

    // replace anything greater than cutoff with cutoff
    replace_if(terms.begin(), terms.end(), greater_than<T>(cutoff), cutoff);
}

#endif