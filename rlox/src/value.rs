use crate::stmt::statement::FunStmt;
use crate::Interpreter;
use std::cmp::Ordering::*;
use std::cmp::{PartialEq, PartialOrd};
use std::ops::{Add, Deref, Div, Mul, Neg, Not, Sub};
use std::rc::Rc;

pub trait Callable {
    fn call(&self, interpreter: &Interpreter, arguments: Vec<LoxValue>) -> LoxValue;
    fn arity(&self) -> usize;
}

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
    fn call(&self, interpreter: &Interpreter, arguments: Vec<LoxValue>) -> LoxValue {
        interpreter.execute_function(arguments, &self.declaration);
        LoxValue::new(Value::Nil)
    }

    fn arity(&self) -> usize {
        self.declaration.parameters.len()
    }
}

#[derive(Clone)]
pub struct LoxValue(pub Rc<Value>);

impl LoxValue {
    pub fn new(value: Value) -> Self {
        Self(Rc::new(value))
    }
}

#[derive(Clone)]
pub enum Value {
    Nil,
    String(String),
    Double(f64),
    Boolean(bool),
    Function(Rc<Box<dyn Callable>>),
}

impl LoxValue {
    pub fn stringify(&self) -> String {
        match self.0.deref() {
            Value::Nil => "nil".to_owned(),
            Value::String(s) => s.clone(),
            Value::Double(d) => {
                format!("{}", d)
            }
            Value::Boolean(b) => format!("{}", b),
            Value::Function(_) => todo!(),
        }
    }

    pub fn is_truthy(&self) -> bool {
        !matches!(self.0.deref(), Value::Nil | Value::Boolean(false))
    }
}

impl Neg for LoxValue {
    type Output = Self;
    fn neg(self) -> Self::Output {
        match self.0.deref() {
            Value::Double(d) => LoxValue(Rc::new(Value::Double(-d))),
            _ => panic!("unsupported to neg {:?}", self.stringify()),
        }
    }
}

impl Not for LoxValue {
    type Output = Self;
    fn not(self) -> Self::Output {
        match self.0.deref() {
            Value::Nil | Value::Boolean(false) => Self(Rc::new(Value::Boolean(true))),
            _ => Self(Rc::new(Value::Boolean(false))),
        }
    }
}

impl Mul<LoxValue> for LoxValue {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        match (self.0.deref(), rhs.0.deref()) {
            (Value::Double(a), Value::Double(b)) => LoxValue(Rc::new(Value::Double(a * b))),
            (_, _) => panic!(
                "literal cannot to mul {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
impl Div<LoxValue> for LoxValue {
    type Output = Self;

    fn div(self, rhs: Self) -> Self::Output {
        match (self.0.deref(), rhs.0.deref()) {
            (Value::Double(a), Value::Double(b)) => LoxValue(Rc::new(Value::Double(a / b))),
            (_, _) => panic!(
                "unsupported to div {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
//
impl Sub<LoxValue> for LoxValue {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        match (self.0.deref(), rhs.0.deref()) {
            (Value::Double(a), Value::Double(b)) => LoxValue(Rc::new(Value::Double(a - b))),
            (_, _) => panic!(
                "unsupported to sub {:?} with {:?}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}
//
impl Add<LoxValue> for LoxValue {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        match (self.0.deref(), rhs.0.deref()) {
            (Value::String(a), Value::String(b)) => {
                LoxValue(Rc::new(Value::String(format!("{}{}", a, b))))
            }
            (Value::String(a), Value::Double(b)) => {
                LoxValue(Rc::new(Value::String(format!("{}{}", a, b))))
            }
            (Value::Double(a), Value::Double(b)) => LoxValue(Rc::new(Value::Double(a + b))),
            (_, _) => panic!(
                "unsupported to add {} with {}",
                self.stringify(),
                rhs.stringify()
            ),
        }
    }
}

// impl Eq for Literal {}

impl PartialEq for LoxValue {
    fn eq(&self, other: &Self) -> bool {
        match (self.0.deref(), other.0.deref()) {
            (Value::Nil, Value::Nil) => true,
            (Value::Nil, _) => false,
            (Value::String(l0), Value::String(r0)) => l0.eq(r0),
            (Value::Double(l0), Value::Double(r0)) => l0.eq(r0),
            (Value::Boolean(l0), Value::Boolean(r0)) => l0.eq(r0),
            _ => panic!("Eq for different type is unsupported"),
        }
    }
}

// impl Ord for Literal {}

impl PartialOrd for LoxValue {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        match (self.0.deref(), other.0.deref()) {
            (Value::Nil, Value::Nil) => Some(Equal),
            (Value::String(a), Value::String(b)) => a.partial_cmp(b),
            (Value::Double(a), Value::Double(b)) => a.partial_cmp(b),
            (Value::Boolean(a), Value::Boolean(b)) => a.partial_cmp(b),

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
