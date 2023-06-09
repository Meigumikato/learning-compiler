use crate::expr::{expression::*, visitor::ExprVisitor};

pub struct AstPrinterVisitor;

impl AstPrinterVisitor {
    #[allow(dead_code)]
    pub fn print(&mut self, expr: &Expr) {
        println!("AST = {}", self.visit_expr(expr));
    }
}

impl ExprVisitor<String> for AstPrinterVisitor {
    fn visit_expr(&self, expr: &Expr) -> String {
        match expr {
            Expr::Nil => todo!(),
            Expr::Literal(l) => l.stringify(),
            Expr::Group(g) => self.visit_group(g),
            Expr::Unary(u) => self.visit_unary(u),
            Expr::Binary(b) => self.visit_binary(b),
            Expr::Ternary(t) => self.visit_ternary(t),
            Expr::Variable(v) => self.visit_variable(v),
            Expr::Assign(_) => todo!(),
            Expr::Logic(_) => todo!(),
            Expr::Call(_) => todo!(),
            Expr::Comma(_) => todo!(),
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

    fn visit_variable(&self, variable: &Variable) -> String {
        variable.name.lexeme.to_string()
    }

    fn visit_assign(&self, _: &Assign) -> String {
        todo!()
    }

    fn visit_logic(&self, _logic: &Logic) -> String {
        todo!()
    }

    fn visit_call(&self, _fun: &Call) -> String {
        todo!()
    }
}
