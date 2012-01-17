#ifndef DIV_L2_HPP_
#define DIV_L2_HPP_
#include "basics.hpp"

#include <vector>

#include "div_func.hpp"

class DivL2 : public DivFunc {
    typedef DivFunc super;

    public:
        DivL2(double ub = .99);

        virtual double operator()(
                const std::vector<float> &rho_x,
                const std::vector<float> &nu_x,
                const std::vector<float> &rho_y,
                const std::vector<float> &nu_y,
                unsigned int dim,
                unsigned int k
            ) const;

    private:
        virtual DivL2* do_clone() const;
};
#endif
