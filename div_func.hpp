#ifndef DIV_FUNC_HPP_
#define DIV_FUNC_HPP_
#include "basics.hpp"

#include <boost/utility.hpp>

#include <vector>

class DivFunc : boost::noncopyable {
    protected:
        double ub; // if ub is .99, will cap terms at the 99-th percentile

    public:
        DivFunc(double ub = .99);

        virtual ~DivFunc() {};

        virtual double operator()(
                const std::vector<float> &rho_x,
                const std::vector<float> &nu_x,
                const std::vector<float> &rho_y,
                const std::vector<float> &nu_y,
                unsigned int dim,
                unsigned int k
            ) const = 0;

        DivFunc* clone() const;

    private:
        virtual DivFunc* do_clone() const = 0;
};

inline DivFunc* new_clone(const DivFunc &df) {
    return df.clone();
}

#endif
