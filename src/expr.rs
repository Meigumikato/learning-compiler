use crate::token::{Literal, Token, TokenType};

// #[derive(Debug)]
// pub trait Expr {
//     // add code here
//
//     fn accept(&self, visitor: &Box<dyn ExprVisitor>) -> String;
// }

pub enum Expr {
    Literal(Literal),
    Unary(Unary),
    Binary(Binary),
    Ternary(Ternary),
    Group(Group),
}

pub trait ExprVisitor {
    fn visit_expr(&self, expr: &Expr) -> String;
    fn visit_unary(&self, unary: &Unary) -> String;
    fn visit_binary(&self, binary: &Binary) -> String;
    fn visit_group(&self, group: &Group) -> String;
    fn visit_ternary(&self, ternary: &Ternary) -> String;
}

pub struct AstPrintVisitor;
pub struct RNPPrintVistior;

impl AstPrintVisitor {
    pub fn new() -> Self {
        Self
    }
}

impl RNPPrintVistior {
    pub fn new() -> Self {
        Self
    }
}

impl ExprVisitor for AstPrintVisitor {
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
            Expr::Ternary(t) => self.visit_ternary(t),
        }
    }

    fn visit_unary(&self, unary: &Unary) -> String {
        format!("{} {}", unary.operator.lexeme, self.visit_expr(&unary.expr))
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

pub struct Unary {
    pub operator: Token,
    pub expr: Box<Expr>, // expr: (dyn Expr),
}

pub struct Binary {
    pub operator: Token,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

pub struct Ternary {
    pub cond: Box<Expr>,
    pub lhs: Box<Expr>,
    pub rhs: Box<Expr>,
}

pub struct Group {
    pub expr: Box<Expr>,
}

// #[cfg(test)]
// mod test {
//     use super::*;
//
//     #[test]
//     fn visitor_pattern() {
//         let u = Unary::new();
//         let b = Binary::new();
//
//         let visitor: Box<dyn ExprVisitor> = Box::new(AstPrintVisitor::new());
//
//         assert!(
//             u.accept(&visitor) == "unary",
//             "AstPrintVisitor Unary {}",
//             u.accept(&visitor)
//         );
//
//         assert!(
//             b.accept(&visitor) == "binary",
//             "AstPrintVisitor Binary {}",
//             b.accept(&visitor)
//         );
//     }
// }
