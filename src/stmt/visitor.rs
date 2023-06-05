use crate::stmt::statement::*;
pub trait StmtVisitor<T> {
    /// .
    fn visit_stmt(&self, stmt: &Stmt) -> T;
    fn visit_var_stmt(&self, var_stmt: &VarStmt) -> T;
    fn visit_expr_stmt(&self, expr_stmt: &ExprStmt) -> T;
    fn visit_print_stmt(&self, print_stmt: &PrintStmt) -> T;
    fn visit_block_stmt(&self, block: &BlockStmt) -> T;
    fn visit_if_stmt(&self, if_stmt: &IfStmt) -> T;
    fn visit_while_stmt(&self, while_stmt: &WhileStmt) -> T;
    fn visit_fun_stmt(&self, fun_stmt: &FunStmt) -> T;
    fn visit_return_stmt(&self, return_stmt: &ReturnStmt) -> T;
    fn visit_break_stmt(&self, break_stmt: &BreakStmt) -> T;
}
