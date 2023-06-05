use std::io::Write;
use std::{env, fs, io};

mod expr;
mod interpreter;
mod parser;
mod scanner;
mod stmt;
mod token;
mod value;

use interpreter::Interpreter;

use crate::parser::Parser;
use crate::scanner::Scanner;

// #[derive(Debug)]
struct Lox {
    scanner: Scanner,
    parser: Parser,
    interpreter: Interpreter,
}

impl Lox {
    fn new() -> Self {
        Self {
            scanner: Scanner::new(),
            parser: Parser::new(),
            interpreter: Interpreter::new(),
        }
    }

    fn run(&mut self, script: &str) {
        self.interpreter
            .interpret(self.parser.parse(self.scanner.scan_tokens(script)));
    }

    fn run_file(&mut self, file_path: &String) {
        let content = fs::read_to_string(file_path).expect("file_path open failed");
        self.run(&content)
    }

    fn run_prompt(&mut self) {
        let stdin = io::stdin();
        loop {
            print!(">> ");
            let _ = io::stdout().flush();
            let mut line = String::new();
            let _ = stdin.read_line(&mut line).expect("read from stdin failed");
            if line.is_empty() {
                break;
            }
            self.run(&line)
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    // println!("args {:?}", args);

    let mut lox = Lox::new();

    match args.len() {
        l if l > 2 => {
            println!("Usage rlox [script]");
        }

        l if l == 2 => {
            lox.run_file(&args[1]);
        }

        _ => {
            lox.run_prompt();
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn scanner_identifiers() {
        let script = include_str!("../test/scanning/identifiers.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn scanner_keywords() {
        let script = include_str!("../test/scanning/keywords.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn scanner_numbers() {
        let script = include_str!("../test/scanning/numbers.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn scanner_punctuators() {
        let script = include_str!("../test/scanning/punctuators.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn scanner_strings() {
        let script = include_str!("../test/scanning/strings.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn scanner_whitespaces() {
        let script = include_str!("../test/scanning/whitespace.lox");
        Lox::new().scanner.scan_tokens(script);
    }

    #[test]
    fn interpreter_statement_1() {
        let script = [
            "print 3 * 5 + 6 * 3 ;",
            "print 2 - 3 + 4 - 5 * 6 ;",
            "print (2 - 3 + 4) * (5 + 6 * 7) ;",
            "print (3 ? 4 : 3 ? 1 ? 2 : 3 : 4) == 4 ;",
            "print 3 ? 4 : 3 ? 1 ? 2 : 3 : 4 == 4 ;",
            "print (3 + 4 + 8) ? 4 : 3 ? 1 ? 2 : 3 : 4 == 4 ;",
            // "print 3 + 4, 5 * 7, 10 * 8 ;",
            "print 3 + 5 ;",
            r#" print true or "nihao" ; "#,
        ];

        let mut lox = Lox::new();
        for s in script {
            lox.run(&s);
        }
    }

    #[test]
    fn interpreter_statement_2() {
        let script = [
            "var a = 10; print a;",
            "var b = \"nihao\"; print b;",
            "var a = 10; var b = \"nihao\"; print b + a;",
            "   var a = 10;
                {
                    // var a = 11;
                    print a;
                    a = a + 1;
                    print a;
                }
                print a;
            ",
            r#"
                var a = "global a";
                var b = "global b";
                var c = "global c";

                {
                    var a = "outer a";
                    var b = "outer b";
                    {
                        var a = "inner a";
                        print a;
                        print b;
                        print c;
                    }
                    print a;
                    print b;
                    print c;
                }

                print a;
                print b;
                print c;
            "#,
        ];

        let mut lox = Lox::new();
        for s in script {
            lox.run(s);
        }
    }

    #[test]
    fn interpreter_statement_3() {
        let script = [
            "for (;false;);",
            r#"fun func(a, b, c) { print "func start"; print a + b +c; } 
             print func(1, 2, 3);"#,
        ];

        let mut lox = Lox::new();
        for s in script {
            lox.run(s);
        }
    }
}
