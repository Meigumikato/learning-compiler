use crate::token::Token;

use crate::value::Value;

use super::visitor::ExprVisitor;

#[allow(dead_code)]
#[derive(Clone)]
pub enum Expr {
    Nil,
    Literal(Value),
    Unary(Unary),
    Binary(Binary),
    Ternary(Ternary),
    Group(Group),
    Variable(Variable),
    Assign(Assign),
    Logic(Logic),
    Call(Call),
}

impl Expr {
    pub fn accept<T>(&self, visitor: &mut impl ExprVisitor<T>) -> T {
        visitor.visit_expr(self)
    }
}

#[derive(Clone)]
pub struct Unary {
    pub operator: Token,
    pub expr: Box<Expr>, // expr: (dyn Expr),
}

#[derive(Clone)]
pub struct Binary {
    pub operator: Token,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

#[derive(Clone)]
pub struct Ternary {
    pub cond: Box<Expr>,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

#[derive(Clone)]
pub struct Group {
    pub expr: Box<Expr>,
}

#[allow(dead_code)]
impl Group {
    pub const LEFT_PARENT: &'static str = "(";
    pub const RIGHT_PARENT: &'static str = ")";
}

#[derive(Clone)]
pub struct Variable {
    pub name: Token,
}

#[derive(Clone)]
pub struct Assign {
    pub name: Token,
    pub value: Box<Expr>,
}

#[derive(Clone)]
pub struct Logic {
    pub operator: Token,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}
//
#[derive(Clone)]
pub struct Call {
    pub callee: Box<Expr>,
    pub paren: Token,
    pub arguments: Vec<Expr>,
}
