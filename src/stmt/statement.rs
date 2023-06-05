use crate::expr::expression::Expr;
use crate::interpreter::Interpreter;
use crate::token::Token;

use super::visitor::StmtVisitor;

#[derive(Clone)]
pub enum Stmt {
    Expr(ExprStmt),
    Print(PrintStmt),
    Var(VarStmt),
    Block(BlockStmt),
    If(IfStmt),
    WhileStmt(WhileStmt),
    FunStmt(FunStmt),
}

impl Stmt {
    pub fn accept(&mut self, interpreter: &mut Interpreter) {
        interpreter.visit_stmt(self);
    }
}

#[derive(Clone)]
pub struct ExprStmt {
    pub expr: Expr,
}

#[derive(Clone)]
pub struct PrintStmt {
    pub expr: Expr,
}

#[derive(Clone)]
pub struct VarStmt {
    pub token: Token,
    pub initializer: Expr,
}

#[derive(Clone)]
pub struct BlockStmt {
    pub statements: Vec<Stmt>,
}

#[derive(Clone)]
pub struct IfStmt {
    pub cond: Expr,
    pub then_branch: Box<Stmt>,
    pub else_branch: Option<Box<Stmt>>,
}

#[derive(Clone)]
pub struct WhileStmt {
    pub cond: Expr,
    pub body: Option<Box<Stmt>>,
}

#[derive(Clone)]
pub struct FunStmt {
    pub name: Token,
    pub parameters: Vec<Token>,
    pub block: Vec<Stmt>,
}
