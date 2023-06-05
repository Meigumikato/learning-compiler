use crate::expr::expression::*;

pub trait ExprVisitor<T> {
    fn visit_expr(&mut self, expr: &Expr) -> T;
    fn visit_unary(&mut self, unary: &Unary) -> T;
    fn visit_binary(&mut self, binary: &Binary) -> T;
    fn visit_group(&mut self, group: &Group) -> T;
    fn visit_ternary(&mut self, ternary: &Ternary) -> T;
    fn visit_variable(&mut self, variable: &Variable) -> T;
    fn visit_assign(&mut self, assign: &Assign) -> T;
    fn visit_logic(&mut self, logic: &Logic) -> T;
    fn visit_call(&mut self, fun: &Call) -> T;
}
