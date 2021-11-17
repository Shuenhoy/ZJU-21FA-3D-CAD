#include <exprtk.hpp>
#include <sweep_expr.hpp>
using T              = double;
using symbol_table_t = exprtk::symbol_table<T>;
using parser_t       = exprtk::parser<T>;

namespace brep_sweep {
Expression::Expression(const std::string &expr) {
    symbol_table_t symbol_table;
    expression = std::make_unique<expression_t>();

    z = std::make_unique<double>(0.0);

    symbol_table.add_variable("z", *z);
    affine.resize(6);

    symbol_table.add_variable("__affine", affine.front());
    symbol_table.add_constants();
    expression->register_symbol_table(symbol_table);
    parser_t parser;

    if (!parser.compile(expr + ";__affine := affine", *expression)) {
        exit(1);
    }
}
Eigen::Vector2d Expression::eval(double z, double x, double y) {
    *this->z = z;
    expression->value();
    Eigen::Matrix2d affine_matrix{
        {affine[0], affine[1]}, {affine[2], affine[3]}};
    Eigen::Vector2d affine_vector{affine[4], affine[5]};

    Eigen::Vector2d point{x, y};
    Eigen::Vector2d result = affine_matrix * point + affine_vector;
    return result;
}
Expression::~Expression()                      = default;
Expression::Expression(Expression &&) noexcept = default;
Expression::Expression(const Expression &) {
    throw std::logic_error("this should not be copied");
};
} // namespace brep_sweep