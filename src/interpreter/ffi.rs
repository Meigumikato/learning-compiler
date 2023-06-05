use crate::value::{Callable, Value};

/// foreign funtion interface -> clock
#[derive(Clone)]
pub struct Clock;

impl Callable for Clock {
    fn call(&mut self, _interpreter: &mut super::Interpreter, _arguments: Vec<Value>) -> Value {
        todo!()
    }

    fn arity(&self) -> usize {
        todo!()
    }
}
