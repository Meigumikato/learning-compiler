// use crate::{
//     expr::{expression::*, visitor::ExprVisitor},
//     value::Value,
// };
//
// use std::{cell::RefCell, collections::VecDeque};
//
// pub struct RNPPrinterVistior {
//     pub stack: RefCell<VecDeque<String>>,
//     pub expr: RefCell<String>,
// }
//
// impl RNPPrinterVistior {
//     #[allow(dead_code)]
//     pub fn generate_rnp(&mut self, expr: &Expr) {
//         self.visit_expr(expr);
//         let mut stack = self.stack.borrow_mut();
//         while !stack.is_empty() {
//             self.expr
//                 .borrow_mut()
//                 .push_str(stack.back().unwrap().as_str());
//
//             stack.pop_back();
//         }
//     }
// }

// impl ExprVisitor<()> for RNPPrinterVistior {
//     fn visit_expr(&mut self, expr: &Expr) {
//         match expr {
//             Expr::Literal(l) => {
//                 let mut expr = self.expr.borrow_mut();
//                 match l {
//                     Value::Nil => {
//                         expr.push_str("nil");
//                     }
//                     Value::String(s) => {
//                         expr.push_str(s.as_str());
//                     }
//                     Value::Double(d) => {
//                         expr.push_str(format!("{}", d).as_str());
//                     }
//                     Value::Boolean(b) => {
//                         expr.push_str(format!("{}", b).as_str());
//                     }
//                     Value::Function(_) => todo!(),
//                 }
//             }
//             Expr::Group(g) => self.visit_group(g),
//             Expr::Unary(u) => self.visit_unary(u),
//             Expr::Binary(b) => self.visit_binary(b),
//             _ => unreachable!(),
//         }
//     }
//
//     fn visit_unary(&mut self, unary: &Unary) {
//         self.expr
//             .borrow_mut()
//             .push_str(unary.operator.lexeme.as_str());
//         self.visit_expr(&unary.expr)
//     }
//
//     fn visit_binary(&mut self, binary: &Binary) {
//         self.visit_expr(&binary.lhs);
//         let op = &binary.operator.lexeme;
//
//         let helper = |curr_op: &str, greater_equal_op: &[&str]| {
//             let mut stack = self.stack.borrow_mut();
//
//             while !stack.is_empty() && greater_equal_op.iter().any(|f| f == stack.back().unwrap()) {
//                 self.expr
//                     .borrow_mut()
//                     .push_str(stack.back().unwrap().as_str());
//
//                 stack.pop_back();
//             }
//
//             stack.push_back(curr_op.to_owned());
//         };
//
//         match op.as_str() {
//             "+" | "-" => {
//                 helper(op.as_str(), &["+", "-", "*", "/"]);
//             }
//
//             "*" | "/" => {
//                 helper(op.as_str(), &["*", "/"]);
//             }
//             _ => {}
//         }
//
//         self.visit_expr(&binary.rhs);
//     }
//
//     fn visit_group(&mut self, group: &Group) {
//         self.stack
//             .borrow_mut()
//             .push_back(Group::LEFT_PARENT.to_owned());
//
//         self.visit_expr(&group.expr);
//
//         let mut stack = self.stack.borrow_mut();
//
//         while stack.back().unwrap() != Group::LEFT_PARENT {
//             self.expr
//                 .borrow_mut()
//                 .push_str(stack.back().unwrap().as_str());
//             stack.pop_back();
//         }
//
//         stack.pop_back();
//     }
//
//     fn visit_ternary(&mut self, _: &Ternary) {
//         todo!()
//     }
//
//     fn visit_variable(&mut self, _: &Variable) {
//         todo!()
//     }
//
//     fn visit_assign(&mut self, _: &Assign) {
//         todo!()
//     }
//
//     fn visit_logic(&mut self, _logic: &Logic) {
//         todo!()
//     }
//
//     fn visit_call(&mut self, _fun: &Call) -> () {
//         todo!()
//     }
// }
