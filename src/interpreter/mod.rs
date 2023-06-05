use crate::expr::visitor::ExprVisitor;
use crate::interpreter::environment::Environment;
use crate::stmt::statement::*;
use crate::stmt::visitor::StmtVisitor;
use crate::{expr::expression::*, value::LoxValue};
use std::cell::RefCell;
use std::ops::Deref;
use std::rc::Rc;

use crate::{token::TokenType, value::Function, value::Value};

pub mod environment;
mod ffi;

// #[derive(Debug)]
pub struct Interpreter {
    global: Rc<RefCell<Environment>>,
    environment: RefCell<Rc<RefCell<Environment>>>,
}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}

pub enum BlockType {
    Normal,
    Function,
    While,
}

pub enum BlockBreak {
    Null,
    Continue,
    LoopBreak,
    FuncReturn(LoxValue),
}

impl Interpreter {
    pub fn new() -> Self {
        // foreign function | native function
        let mut global = Environment::new(None);
        let clock = LoxValue::new(Value::Function(Rc::new(Box::new(ffi::Clock {}))));
        global.define("clock", &clock);

        Self {
            global: Rc::new(RefCell::new(global)),
            environment: RefCell::new(Rc::new(RefCell::new(Environment::new(None)))),
        }
    }

    #[allow(dead_code)]
    pub fn evaluate(&self, expr: &Expr) -> LoxValue {
        expr.accept(self)
    }

    fn execute(&self, stmt: &Stmt) -> BlockBreak {
        stmt.accept(self)
    }

    fn execute_block(
        &self,
        block_type: BlockType,
        stmts: &[Stmt],
        environment: Rc<RefCell<Environment>>,
    ) {
        // enter child scope
        let parent_scope = self.environment.replace(environment);

        'block: for stmt in stmts {
            let block_break = self.execute(stmt);

            match (&block_type, &block_break) {
                (BlockType::Function, BlockBreak::FuncReturn(_)) => break 'block,
                // (BlockType::While, BlockBreak::LoopBreak) => break 'block,
                _ => {}
            }
        }

        // back parent scope
        self.environment.replace(parent_scope);
    }

    pub fn execute_function(&self, arguments: Vec<LoxValue>, stmt: &FunStmt) {
        let mut fun_scope = Environment::new(Some(self.global.clone()));
        for (i, argument) in arguments.iter().enumerate() {
            fun_scope.define(&stmt.parameters[i].lexeme, argument)
        }

        self.execute_block(
            BlockType::Function,
            &stmt.block,
            Rc::new(RefCell::new(fun_scope)),
        )
    }

    pub fn interpret(&mut self, stmts: Vec<Stmt>) {
        for mut stmt in stmts {
            self.execute(&mut stmt);
        }
    }

    #[allow(dead_code)]
    pub fn interpret_repl(&mut self, stmts: Vec<Stmt>) {
        for stmt in stmts {
            stmt.accept(self);
        }
    }
}

impl ExprVisitor<LoxValue> for Interpreter {
    fn visit_expr(&self, expr: &Expr) -> LoxValue {
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
            Expr::Comma(_) => todo!(),
        }
    }

    fn visit_unary(&self, unary: &Unary) -> LoxValue {
        match unary.operator.token_type {
            TokenType::Bang => !self.visit_expr(&unary.expr),
            TokenType::Minus => -self.visit_expr(&unary.expr),
            _ => panic!("unsupported unary operator {:?}", unary.operator.lexeme),
        }
    }

    fn visit_binary(&self, binary: &Binary) -> LoxValue {
        let left_literal = self.visit_expr(&binary.lhs);
        let right_literal = self.visit_expr(&binary.rhs);

        use TokenType::*;

        let helper_gen_boolean = |value: bool| LoxValue::new(Value::Boolean(value));

        match binary.operator.token_type {
            Plus => left_literal + right_literal,
            Minus => left_literal - right_literal,
            Star => left_literal * right_literal,
            Slash => left_literal / right_literal,

            Comma => right_literal,

            EqualEqual => helper_gen_boolean(left_literal == right_literal),
            BangEqual => helper_gen_boolean(left_literal != right_literal),
            Greater => helper_gen_boolean(left_literal > right_literal),
            GreaterEqual => helper_gen_boolean(left_literal >= right_literal),
            Less => helper_gen_boolean(left_literal < right_literal),
            LessEqual => helper_gen_boolean(left_literal <= right_literal),

            _ => panic!("unsupported binary operator {:?}", binary.operator.lexeme),
        }
    }

    fn visit_group(&self, group: &Group) -> LoxValue {
        self.visit_expr(&group.expr)
    }

    fn visit_ternary(&self, ternary: &Ternary) -> LoxValue {
        match self.visit_expr(&ternary.cond).0.deref() {
            Value::Boolean(b) => {
                if *b {
                    self.visit_expr(&ternary.lhs)
                } else {
                    self.visit_expr(&ternary.rhs)
                }
            }
            Value::Double(d) => {
                if d.gt(&0f64) {
                    self.visit_expr(&ternary.lhs)
                } else {
                    self.visit_expr(&ternary.rhs)
                }
            }
            _ => panic!("only allow ternary cond is Boolean or Number"),
        }
    }

    fn visit_variable(&self, variable: &Variable) -> LoxValue {
        self.environment
            .borrow()
            .borrow()
            .get(&variable.name.lexeme)
    }

    fn visit_assign(&self, assign: &Assign) -> LoxValue {
        let value = self.evaluate(&assign.value);

        self.environment
            .borrow_mut()
            .borrow_mut()
            .assign(&assign.name.lexeme, &value);

        value
    }

    fn visit_logic(&self, logic: &Logic) -> LoxValue {
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

    fn visit_call(&self, fun: &Call) -> LoxValue {
        let callee = fun.callee.accept(self);

        let mut arguments = vec![];

        for argument in &fun.arguments {
            arguments.push(argument.accept(self));
        }

        match callee.0.deref() {
            Value::Function(fun) => {
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

impl StmtVisitor<BlockBreak> for Interpreter {
    fn visit_stmt(&self, stmt: &Stmt) -> BlockBreak {
        match stmt {
            Stmt::Expr(es) => self.visit_expr_stmt(es),
            Stmt::Print(ps) => self.visit_print_stmt(ps),
            Stmt::Var(vs) => self.visit_var_stmt(vs),
            Stmt::Block(bs) => self.visit_block_stmt(bs),
            Stmt::If(i) => self.visit_if_stmt(i),
            Stmt::While(w) => self.visit_while_stmt(w),
            Stmt::Fun(f) => self.visit_fun_stmt(f),
            Stmt::Return(r) => self.visit_return_stmt(r),
            Stmt::Break(b) => self.visit_break_stmt(b),
        }
    }

    fn visit_var_stmt(&self, var_stmt: &VarStmt) -> BlockBreak {
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
            .borrow_mut()
            .define(&var_stmt.token.lexeme, &value);

        BlockBreak::Null
    }

    fn visit_expr_stmt(&self, expr_stmt: &ExprStmt) -> BlockBreak {
        self.evaluate(&expr_stmt.expr);
        BlockBreak::Null
    }

    fn visit_print_stmt(&self, print_stmt: &PrintStmt) -> BlockBreak {
        let val = self.evaluate(&print_stmt.expr);
        println!("{}", val.stringify());
        BlockBreak::Null
    }

    fn visit_block_stmt(&self, block: &BlockStmt) -> BlockBreak {
        let block_scope = Rc::new(RefCell::new(Environment::new(Some(
            self.environment.borrow().clone(),
        ))));
        self.execute_block(BlockType::Normal, &block.statements, block_scope);
        BlockBreak::Null
    }

    fn visit_if_stmt(&self, if_stmt: &IfStmt) -> BlockBreak {
        let cond = self.evaluate(&if_stmt.cond).is_truthy();

        if cond {
            if_stmt.then_branch.accept(self)
        } else if let Some(ref el) = if_stmt.else_branch {
            el.accept(self)
        } else {
            BlockBreak::Null
        }
    }

    fn visit_while_stmt(&self, while_stmt: &WhileStmt) -> BlockBreak {
        while self.evaluate(&while_stmt.cond).is_truthy() {
            if let Some(ref b) = while_stmt.body {
                let while_scope = Rc::new(RefCell::new(Environment::new(Some(
                    self.environment.borrow().clone(),
                ))));

                match b.deref() {
                    Stmt::Block(block) => {
                        self.execute_block(BlockType::While, &block.statements, while_scope)
                    }
                    _ => {
                        panic!("WhileStmt Body is Not block");
                    }
                }

                // b.accept(self);
            }
        }
        BlockBreak::Null
    }

    fn visit_fun_stmt(&self, fun_stmt: &FunStmt) -> BlockBreak {
        let function = LoxValue::new(Value::Function(Rc::new(Box::new(Function::new(
            fun_stmt.clone(),
        )))));
        self.environment
            .borrow_mut()
            .borrow_mut()
            .define(&fun_stmt.name.lexeme, &function);

        BlockBreak::Null
    }

    fn visit_return_stmt(&self, return_stmt: &ReturnStmt) -> BlockBreak {
        let ret_value;
        if let Some(ref expr) = return_stmt.expr {
            ret_value = expr.accept(self);
        } else {
            ret_value = LoxValue::new(Value::Nil);
        }
        BlockBreak::FuncReturn(ret_value)
    }

    fn visit_break_stmt(&self, _break_stmt: &BreakStmt) -> BlockBreak {
        BlockBreak::LoopBreak
    }
}
