use std::{cell::RefCell, collections::HashMap, rc::Rc};

use crate::value::LoxValue;

#[derive(Clone)]
pub struct Environment {
    enclosing: Option<Rc<RefCell<Environment>>>,
    values: HashMap<String, LoxValue>,
}

impl Environment {
    pub fn new(enclosing: Option<Rc<RefCell<Environment>>>) -> Self {
        match enclosing {
            Some(en) => Self {
                enclosing: Some(en),
                values: HashMap::new(),
            },
            None => Self {
                enclosing: None,
                values: HashMap::new(),
            },
        }
    }

    pub fn define(&mut self, name: &str, value: &LoxValue) {
        self.values.insert(name.to_owned(), value.clone());
    }

    pub fn get(&self, name: &str) -> LoxValue {
        if let Some(value) = self.values.get(name) {
            return value.clone();
        }

        if let Some(ref en) = self.enclosing {
            return en.borrow().get(name);
        }

        panic!("Undefined varibale '{}'", name);
    }

    pub fn assign(&mut self, name: &str, value: &LoxValue) {
        if self.values.contains_key(name) {
            self.values.get_mut(name).unwrap().clone_from(value);
            return;
        }

        if let Some(ref mut en) = self.enclosing {
            en.borrow_mut().assign(name, value);
            return;
        }

        panic!("Undefined varibale '{}' .", name)
    }
}
