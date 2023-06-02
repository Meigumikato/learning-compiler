use std::cmp::Ordering::*;
use std::cmp::{Eq, PartialEq, PartialOrd};
use std::ops::{Add, Div, Mul, Neg, Not, Sub};

#[derive(Debug, Clone)]
pub enum Literal {
    Nil,
    String(String),
    Double(f64),
    Boolean(bool),
}

impl Neg for Literal {
    type Output = Self;
    fn neg(self) -> Self::Output {
        match self {
            Self::Double(d) => Self::Double(-d),
            _ => panic!("unsupported to neg {:?}", self),
        }
    }
}

impl Not for Literal {
    type Output = Self;
    fn not(self) -> Self::Output {
        match self {
            Self::Nil | Self::Boolean(false) => Self::Boolean(true),
            _ => Self::Boolean(false),
        }
    }
}

impl Mul<Literal> for Literal {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a * b),
            (_, _) => panic!("literal cannot to mul {:?} with {:?}", self, rhs),
        }
    }
}
impl Div<Literal> for Literal {
    type Output = Self;

    fn div(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a / b),
            (_, _) => panic!("unsupported to div {:?} with {:?}", self, rhs),
        }
    }
}
//
impl Sub<Literal> for Literal {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::Double(a), Self::Double(b)) => Self::Double(a - b),
            (_, _) => panic!("unsupported to sub {:?} with {:?}", self, rhs),
        }
    }
}
//
impl Add<Literal> for Literal {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        match (&self, &rhs) {
            (Self::String(a), Self::String(b)) => Self::String(format!("{}{}", a, b)),
            (Self::String(a), Self::Double(b)) => Self::String(format!("{}{}", a, b)),
            (Self::Double(a), Self::Double(b)) => Self::Double(a + b),
            (_, _) => panic!("unsupported to add {:?} with {:?}", self, rhs),
        }
    }
}

// impl Eq for Literal {}

impl PartialEq for Literal {
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

impl PartialOrd for Literal {
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
