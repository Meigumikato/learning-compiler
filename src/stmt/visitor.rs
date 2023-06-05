use crate::stmt::statement::*;
pub trait StmtVisitor<T> {
    /// .
    fn visit_stmt(&mut self, stmt: &mut Stmt);
    fn visit_var_stmt(&mut self, var_stmt: &mut VarStmt);
    fn visit_expr_stmt(&mut self, expr_stmt: &mut ExprStmt);
    fn visit_print_stmt(&mut self, print_stmt: &mut PrintStmt);
    fn visit_block_stmt(&mut self, block: &mut BlockStmt);
    fn visit_if_stmt(&mut self, if_stmt: &mut IfStmt);
    fn visit_while_stmt(&mut self, while_stmt: &mut WhileStmt);
    fn visit_fun_stmt(&mut self, fun_stmt: &mut FunStmt);
}
