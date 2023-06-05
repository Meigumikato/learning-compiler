use crate::expr::visitor::ExprVisitor;
use crate::interpreter::environment::Environment;
use crate::stmt::statement::*;
use crate::stmt::visitor::StmtVisitor;
use crate::{expr::expression::*, value};
use std::cell::RefCell;
use std::rc::Rc;

use crate::{token::TokenType, value::Value};

pub mod environment;
mod ffi;

// #[derive(Debug)]
pub struct Interpreter {
    global: Rc<RefCell<Environment>>,
    environment: Rc<RefCell<Environment>>,
}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}

impl Interpreter {
    pub fn new() -> Self {
        // foreign function | native function
        let mut global = Environment::new(None);
        global.define("clock", &Value::Function(Box::new(ffi::Clock {})));

        Self {
            global: Rc::new(RefCell::new(global)),
            environment: Rc::new(RefCell::new(Environment::new(None))),
        }
    }

    #[allow(dead_code)]
    pub fn evaluate(&mut self, expr: &Expr) -> Value {
        expr.accept(self)
    }

    fn execute(&mut self, stmt: &mut Stmt) {
        stmt.accept(self);
    }

    fn execute_block(&mut self, stmts: &mut [Stmt], environment: Rc<RefCell<Environment>>) {
        let child_scope = environment;

        let parent_scope = self.environment.clone();

        // enter child scope
        self.environment = child_scope;

        for stmt in stmts {
            self.execute(stmt);
        }

        // back parent scope
        self.environment = parent_scope;
    }

    pub fn execute_function(&mut self, arguments: Vec<Value>, stmt: &mut FunStmt) {
        let mut fun_scope = Environment::new(Some(self.global.clone()));
        for (i, argument) in arguments.iter().enumerate() {
            fun_scope.define(&stmt.parameters[i].lexeme, argument)
        }

        self.execute_block(&mut stmt.block, Rc::new(RefCell::new(fun_scope)))
    }

    pub fn interpret(&mut self, stmts: Vec<Stmt>) {
        for mut stmt in stmts {
            self.execute(&mut stmt);
        }
    }

    #[allow(dead_code)]
    pub fn interpret_repl(&mut self, stmts: Vec<Stmt>) {
        for mut stmt in stmts {
            stmt.accept(self);
        }
    }
}

impl ExprVisitor<Value> for Interpreter {
    fn visit_expr(&mut self, expr: &Expr) -> Value {
        match expr {
            Expr::Nil => panic!("can not visit nil expression"),
            Expr::Literal(l) => l.clone(),
            Expr::Group(g) => self.visit_group(g),
            Expr::Unary(u) => self.visit_unary(u),
            Expr::Binary(b) => self.visit_binary(b),
            Expr::Ternary(t) => self.visit_ternary(t),
            Expr::Variable(v) => self.visit_variable(v),
            Expr::Assign(a) => self.visit_assign(a),
            Expr::Logic(l) => self.visit_logic(l),
            Expr::Call(c) => self.visit_call(c),
        }
    }

    fn visit_unary(&mut self, unary: &Unary) -> Value {
        match unary.operator.token_type {
            TokenType::Bang => !self.visit_expr(&unary.expr),
            TokenType::Minus => -self.visit_expr(&unary.expr),
            _ => panic!("unsupported unary operator {:?}", unary.operator.lexeme),
        }
    }

    fn visit_binary(&mut self, binary: &Binary) -> Value {
        let left_literal = self.visit_expr(&binary.lhs);
        let right_literal = self.visit_expr(&binary.rhs);

        use TokenType::*;
        match binary.operator.token_type {
            Plus => left_literal + right_literal,
            Minus => left_literal - right_literal,
            Star => left_literal * right_literal,
            Slash => left_literal / right_literal,

            Comma => right_literal,

            EqualEqual => Value::Boolean(left_literal == right_literal),
            BangEqual => Value::Boolean(left_literal != right_literal),
            Greater => Value::Boolean(left_literal > right_literal),
            GreaterEqual => Value::Boolean(left_literal >= right_literal),
            Less => Value::Boolean(left_literal < right_literal),
            LessEqual => Value::Boolean(left_literal <= right_literal),

            _ => panic!("unsupported binary operator {:?}", binary.operator.lexeme),
        }
    }

    fn visit_group(&mut self, group: &Group) -> Value {
        self.visit_expr(&group.expr)
    }

    fn visit_ternary(&mut self, ternary: &Ternary) -> Value {
        match self.visit_expr(&ternary.cond) {
            Value::Boolean(b) => {
                if b {
                    self.visit_expr(&ternary.lhs)
                } else {
                    self.visit_expr(&ternary.rhs)
                }
            }
            Value::Double(d) => {
                if d > 0f64 {
                    self.visit_expr(&ternary.lhs)
                } else {
                    self.visit_expr(&ternary.rhs)
                }
            }
            _ => panic!("only allow ternary cond is Boolean or Number"),
        }
    }

    fn visit_variable(&mut self, variable: &Variable) -> Value {
        self.environment.borrow().get(&variable.name.lexeme)
    }

    fn visit_assign(&mut self, assign: &Assign) -> Value {
        let value = self.evaluate(&assign.value);

        self.environment
            .borrow_mut()
            .assign(&assign.name.lexeme, &value);

        value
    }

    fn visit_logic(&mut self, logic: &Logic) -> Value {
        let value = self.evaluate(&logic.lhs);

        if logic.operator.token_type == TokenType::Or {
            if value.is_truthy() {
                return value;
            }
        } else if logic.operator.token_type == TokenType::And && !value.is_truthy() {
            return value;
        }

        self.evaluate(&logic.rhs)
    }

    fn visit_call(&mut self, fun: &Call) -> Value {
        let callee = fun.callee.accept(self);

        let mut arguments = vec![];

        for argument in &fun.arguments {
            arguments.push(argument.accept(self));
        }

        match callee {
            Value::Function(mut fun) => {
                if fun.arity() != arguments.len() {
                    panic!("")
                }
                fun.call(self, arguments)
            }
            _ => {
                panic!("")
            }
        }
    }
}

impl StmtVisitor<()> for Interpreter {
    fn visit_stmt(&mut self, stmt: &mut Stmt) {
        match stmt {
            Stmt::Expr(es) => self.visit_expr_stmt(es),
            Stmt::Print(ps) => self.visit_print_stmt(ps),
            Stmt::Var(vs) => self.visit_var_stmt(vs),
            Stmt::Block(bs) => self.visit_block_stmt(bs),
            Stmt::If(i) => self.visit_if_stmt(i),
            Stmt::WhileStmt(w) => self.visit_while_stmt(w),
            Stmt::FunStmt(f) => self.visit_fun_stmt(f),
        }
    }

    fn visit_var_stmt(&mut self, var_stmt: &mut VarStmt) {
        let value;
        match var_stmt.initializer {
            Expr::Nil => {
                panic!("")
            }
            _ => {
                value = self.evaluate(&var_stmt.initializer);
            }
        }

        self.environment
            .borrow_mut()
            .define(&var_stmt.token.lexeme, &value);
    }

    fn visit_expr_stmt(&mut self, expr_stmt: &mut ExprStmt) {
        self.evaluate(&expr_stmt.expr);
    }

    fn visit_print_stmt(&mut self, print_stmt: &mut PrintStmt) {
        let val = self.evaluate(&print_stmt.expr);
        println!("{}", val.stringify());
    }

    fn visit_block_stmt(&mut self, block: &mut BlockStmt) {
        self.execute_block(
            block.statements.as_mut(),
            Rc::new(RefCell::new(Environment::new(Some(
                self.environment.clone(),
            )))),
        );
    }

    fn visit_if_stmt(&mut self, if_stmt: &mut IfStmt) {
        let cond = self.evaluate(&if_stmt.cond).is_truthy();

        if cond {
            if_stmt.then_branch.accept(self);
        } else if let Some(ref mut el) = if_stmt.else_branch {
            el.accept(self);
        }
    }

    fn visit_while_stmt(&mut self, while_stmt: &mut WhileStmt) {
        while self.evaluate(&while_stmt.cond).is_truthy() {
            if let Some(ref mut b) = while_stmt.body {
                b.accept(self);
            }
        }
    }

    fn visit_fun_stmt(&mut self, fun_stmt: &mut FunStmt) {
        let function = value::Function::new(fun_stmt.clone());
        self.environment
            .borrow_mut()
            .define(&fun_stmt.name.lexeme, &Value::Function(Box::new(function)))
    }
}
