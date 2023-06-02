use crate::token::{Token, TokenType};
use crate::value::Literal;

use std::cell::RefCell;
use std::collections::VecDeque;

#[derive(Debug)]
pub enum Expr {
    Literal(Literal),
    Unary(Unary),
    Binary(Binary),
    // Ternary(Ternary),
    Group(Group),
}

impl Expr {
    // pub fn execute(&self, interpret: &Interpreter) -> Literal {
    //     interpret.visit_expr(self)
    // }
}

#[derive(Debug)]
pub struct Unary {
    pub operator: Token,
    pub expr: Box<Expr>, // expr: (dyn Expr),
}

#[derive(Debug)]
pub struct Binary {
    pub operator: Token,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

#[derive(Debug)]
pub struct Ternary {
    pub cond: Box<Expr>,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

#[derive(Debug)]
pub struct Group {
    pub expr: Box<Expr>,
}

pub trait ExprVisitor<T> {
    fn visit_expr(&self, expr: &Expr) -> T;
    fn visit_unary(&self, unary: &Unary) -> T;
    fn visit_binary(&self, binary: &Binary) -> T;
    fn visit_group(&self, group: &Group) -> T;
    fn visit_ternary(&self, ternary: &Ternary) -> T;
}

pub struct AstPrintVisitor;
pub struct RNPPrintVistior {
    pub stack: RefCell<VecDeque<String>>,
    pub expr: String,
}
pub struct Interpreter;

impl ExprVisitor<Literal> for Interpreter {
    fn visit_expr(&self, expr: &Expr) -> Literal {
        match expr {
            Expr::Literal(l) => l.clone(),
            Expr::Group(g) => self.visit_group(g),
            Expr::Unary(u) => self.visit_unary(u),
            Expr::Binary(b) => self.visit_binary(b),
        }
        // todo!()
    }

    fn visit_unary(&self, unary: &Unary) -> Literal {
        match unary.operator.token_type {
            TokenType::Bang => !self.visit_expr(&unary.expr),
            TokenType::Minus => -self.visit_expr(&unary.expr),
            _ => panic!("unsupported unary operator {:?}", unary.operator),
        }
    }

    fn visit_binary(&self, binary: &Binary) -> Literal {
        match binary.operator.token_type {
            TokenType::Plus => self.visit_expr(&binary.lhs) + self.visit_expr(&binary.rhs),
            TokenType::Minus => self.visit_expr(&binary.lhs) - self.visit_expr(&binary.rhs),
            TokenType::Star => self.visit_expr(&binary.lhs) * self.visit_expr(&binary.rhs),
            TokenType::Slash => self.visit_expr(&binary.lhs) / self.visit_expr(&binary.rhs),
            TokenType::EqualEqual => {
                Literal::Boolean(self.visit_expr(&binary.lhs) == self.visit_expr(&binary.rhs))
            }
            TokenType::BangEqual => {
                Literal::Boolean(self.visit_expr(&binary.lhs) != self.visit_expr(&binary.rhs))
            }
            TokenType::Greater => {
                Literal::Boolean(self.visit_expr(&binary.lhs) > self.visit_expr(&binary.rhs))
            }
            TokenType::GreaterEqual => {
                Literal::Boolean(self.visit_expr(&binary.lhs) >= self.visit_expr(&binary.rhs))
            }
            TokenType::Less => {
                Literal::Boolean(self.visit_expr(&binary.lhs) < self.visit_expr(&binary.rhs))
            }
            TokenType::LessEqual => {
                Literal::Boolean(self.visit_expr(&binary.lhs) <= self.visit_expr(&binary.rhs))
            }
            _ => panic!("unsupported binary operator {:?}", binary.operator),
        }
    }

    fn visit_group(&self, group: &Group) -> Literal {
        self.visit_expr(&group.expr)
    }

    fn visit_ternary(&self, _: &Ternary) -> Literal {
        todo!()
    }
}

impl ExprVisitor<String> for AstPrintVisitor {
    fn visit_expr(&self, expr: &Expr) -> String {
        match expr {
            Expr::Literal(l) => match l {
                Literal::Nil => format!("nil"),
                Literal::String(s) => format!("{}", s),
                Literal::Double(d) => format!("{}", d),
                Literal::Boolean(b) => format!("{}", b),
            },
            Expr::Group(g) => self.visit_group(g),
            Expr::Unary(u) => self.visit_unary(u),
            Expr::Binary(b) => self.visit_binary(b),
            // Expr::Ternary(t) => self.visit_ternary(t),
        }
    }

    fn visit_unary(&self, unary: &Unary) -> String {
        format!("{}{}", unary.operator.lexeme, self.visit_expr(&unary.expr))
    }

    fn visit_binary(&self, binary: &Binary) -> String {
        format!(
            "( {} {} {} )",
            binary.operator.lexeme,
            self.visit_expr(&binary.lhs),
            self.visit_expr(&binary.rhs)
        )
    }

    fn visit_group(&self, group: &Group) -> String {
        format!("(group {} )", self.visit_expr(&group.expr))
    }

    fn visit_ternary(&self, ternary: &Ternary) -> String {
        format!(
            "( {} ? {} : {} )",
            self.visit_expr(&ternary.cond),
            self.visit_expr(&ternary.lhs),
            self.visit_expr(&ternary.rhs)
        )
    }
}

impl ExprVisitor<()> for RNPPrintVistior {
    fn visit_expr(&self, expr: &Expr) {
        match expr {
            Expr::Literal(l) => match l {
                Literal::Nil => {
                    self.stack.borrow_mut().push_back("nil".to_string());
                }
                Literal::String(s) => {
                    self.stack.borrow_mut().push_back(s.clone());
                }
                Literal::Double(d) => {
                    self.stack.borrow_mut().push_back(format!("{}", d));
                }
                Literal::Boolean(b) => {
                    self.stack.borrow_mut().push_back(format!("{}", b));
                }
            },
            Expr::Group(g) => self.visit_group(g),
            Expr::Unary(u) => self.visit_unary(u),
            Expr::Binary(b) => self.visit_binary(b),
            // Expr::Ternary(t) => self.visit_ternary(t),
        }
        //todo!()
    }

    fn visit_unary(&self, unary: &Unary) {
        self.visit_expr(&unary.expr)
    }

    fn visit_binary(&self, binary: &Binary) {
        self.visit_expr(&binary.lhs);

        self.visit_expr(&binary.rhs);
    }

    fn visit_group(&self, group: &Group) {
        self.visit_expr(&group.expr);
    }

    fn visit_ternary(&self, _: &Ternary) {
        todo!()
    }
}
