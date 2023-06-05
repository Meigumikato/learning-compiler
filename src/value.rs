use dyn_clone::DynClone;

use crate::stmt::statement::FunStmt;
use crate::Interpreter;
use std::any::Any;
use std::cmp::Ordering::*;
use std::cmp::{PartialEq, PartialOrd};
use std::ops::{Add, Div, Mul, Neg, Not, Sub};

pub trait Callable: DynClone {
    fn call(&mut self, interpreter: &mut Interpreter, arguments: Vec<Value>) -> Value;
    fn arity(&self) -> usize;
}

dyn_clone::clone_trait_object!(Callable);

#[derive(Clone)]
pub struct Function {
    declaration: FunStmt,
}

impl Function {
    pub fn new(fun_stmt: FunStmt) -> Self {
        Self {
            declaration: fun_stmt,
        }
    }
}

impl Callable for Function {
    fn call(&mut self, interpreter: &mut Interpreter, arguments: Vec<Value>) -> Value {
        interpreter.execute_function(arguments, &mut self.declaration);
        Value::Nil
    }

    fn arity(&self) -> usize {
        self.declaration.parameters.len()
    }
}

#[derive(Clone)]
pub enum Value {
    Nil,
    String(String),
    Double(f64),
    Boolean(bool),
    Function(Box<dyn Callable>),
}

impl Value {
    pub fn stringify(&self) -> String {
        match self {
            Self::Nil => "nil".to_owned(),
            Self::String(s) => s.clone(),
            Self::Double(d) => {
                format!("{}", d)
            }
            Self::Boolean(b) => format!("{}", b),
            Value::Function(_) => todo!(),
        }
    }

    pub fn is_truthy(&self) -> bool {
        !matches!(self, Self::Nil | Self::Boolean(false))
    }
}

impl Neg for Value {
    type Output = Self;
    fn neg(self) -> Self::Output {
        match self {
            Self::Double(d) => Self::Double(-d),
            _ => panic!("unsupported to neg {:?}", self.stringify()),
        }
    }
}

impl Not for Value {
    type Output = Self;
    fn not(self) -> Self::Output {
        match self {
            Self::Nil | Self::Boolean(false) => Self::Boolean(true),
            _ => Self::Boolean(false),
        }
    }
}

impl Mul<Value> for Value {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a * b),
            (_, _) => panic!(
                "literal cannot to mul {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
impl Div<Value> for Value {
    type Output = Self;

    fn div(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a / b),
            (_, _) => panic!(
                "unsupported to div {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
//
impl Sub<Value> for Value {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a - b),
            (_, _) => panic!(
                "unsupported to sub {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
//
impl Add<Value> for Value {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::String(a), Self::String(b)) => Self::String(format!("{}{}", a, b)),
            (Self::String(a), Self::Double(b)) => Self::String(format!("{}{}", a, b)),
            (Self::Double(a), Self::Double(b)) => Self::Double(a + b),
            (_, _) => panic!(
                "unsupported to add {} with {}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}

// impl Eq for Literal {}

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::Nil, Self::Nil) => true,
            (Self::Nil, _) => false,
            (Self::String(l0), Self::String(r0)) => l0.eq(r0),
            (Self::Double(l0), Self::Double(r0)) => l0.eq(r0),
            (Self::Boolean(l0), Self::Boolean(r0)) => l0.eq(r0),
            _ => panic!("Eq for different type is unsupported"),
        }
    }
}

// impl Ord for Literal {}

impl PartialOrd for Value {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        match (self, other) {
            (Self::Nil, Self::Nil) => Some(Equal),
            (Self::String(a), Self::String(b)) => a.partial_cmp(b),
            (Self::Double(a), Self::Double(b)) => a.partial_cmp(b),
            (Self::Boolean(a), Self::Boolean(b)) => a.partial_cmp(b),

            _ => panic!("cann't compare"),
        }
    }

    fn lt(&self, other: &Self) -> bool {
        matches!(self.partial_cmp(other), Some(Less))
    }

    fn le(&self, other: &Self) -> bool {
        matches!(self.partial_cmp(other), Some(Less | Equal))
    }

    fn gt(&self, other: &Self) -> bool {
        matches!(self.partial_cmp(other), Some(Greater))
    }

    fn ge(&self, other: &Self) -> bool {
        matches!(self.partial_cmp(other), Some(Greater | Equal))
    }
}
