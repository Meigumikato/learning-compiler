use crate::expr::expression::*;

pub trait ExprVisitor<T> {
    fn visit_expr(&self, expr: &Expr) -> T;
    fn visit_unary(&self, unary: &Unary) -> T;
    fn visit_binary(&self, binary: &Binary) -> T;
    fn visit_group(&self, group: &Group) -> T;
    fn visit_ternary(&self, ternary: &Ternary) -> T;
    fn visit_variable(&self, variable: &Variable) -> T;
    fn visit_assign(&self, assign: &Assign) -> T;
    fn visit_logic(&self, logic: &Logic) -> T;
    fn visit_call(&self, fun: &Call) -> T;
}
