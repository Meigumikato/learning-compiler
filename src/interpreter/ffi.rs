use crate::value::{Callable, LoxValue};

/// foreign funtion interface -> clock
#[derive(Clone)]
pub struct Clock;

impl Callable for Clock {
    fn call(&self, _interpreter: &super::Interpreter, _arguments: Vec<LoxValue>) -> LoxValue {
        todo!()
    }

    fn arity(&self) -> usize {
        todo!()
    }
}
