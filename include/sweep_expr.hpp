#pragma once

#ifndef SWEEP_EXPR_HPP_
#define SWEEP_EXPR_HPP_

#include <exprtk.hpp>
#include <Eigen/Core>

#include <memory>

namespace exprtk {
template <typename T>
class expression;
}

namespace brep_sweep {
using expression_t = exprtk::expression<double>;

struct Expression {

    std::unique_ptr<double> z;
    std::vector<double> affine;
    std::unique_ptr<expression_t> expression;

    Expression(const std::string &expr);

    Expression(const Expression &);
    Expression(Expression &&) noexcept;
    ~Expression();

    Eigen::Vector2d eval(double z, double x, double y);
};

} // namespace brep_sweep
#endif