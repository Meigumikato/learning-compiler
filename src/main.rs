use std::{env, fs, io};

mod expr;
mod parser;
mod scanner;
mod token;

use crate::parser::Parser;
use crate::scanner::Scanner;
// use crate::token::Token;

// fn run_test(script: &String) -> Vec<Token> {
//     let scanner = Scanner::new(script.clone());
//     scanner.scan_tokens()
// }

fn run(script: &String) {
    let scanner = Scanner::new(script.clone());
    let tokens = scanner.scan_tokens();

    let mut parser = Parser::new(tokens);

    parser.parse();
}

fn run_file(file_path: &String) {
    let content = fs::read_to_string(file_path).expect("file_path open failed");
    run(&content)
}

fn run_prompt() {
    let stdin = io::stdin();
    let mut line = String::new();
    loop {
        println!("> ");
        let _ = stdin.read_line(&mut line).expect("read from stdin failed");
        if line.is_empty() {
            break;
        }
        run(&line)
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    println!("args {:?}", args);

    if args.len() > 2 {
        println!("Usage rlox [script]");
    } else if args.len() == 2 {
        run_file(&args[1]);
    } else {
        run_prompt();
    }
}

#[cfg(test)]
mod test {

    use crate::expr::ExprVisitor;

    use super::*;

    #[test]
    fn scanner_identifiers() {
        let script = include_str!("../test/scanning/identifiers.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn scanner_keywords() {
        let script = include_str!("../test/scanning/keywords.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn scanner_numbers() {
        let script = include_str!("../test/scanning/numbers.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn scanner_punctuators() {
        let script = include_str!("../test/scanning/punctuators.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn scanner_strings() {
        let script = include_str!("../test/scanning/strings.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn scanner_whitespaces() {
        let script = include_str!("../test/scanning/whitespace.lox");
        let scanner = Scanner::new(script.to_string());
        let _ = scanner.scan_tokens();
    }

    #[test]
    fn parser_expressions() {
        use crate::expr::AstPrintVisitor;
        let script = include_str!("../test/expressions/parse.lox");
        let scanner = Scanner::new(script.to_string());
        let tokens = scanner.scan_tokens();

        let mut parser = Parser::new(tokens);

        let printer = AstPrintVisitor::new();

        // "group (Double(5.0)-group (Double(3.0)-Double(1.0)))+-Double(1.0)"
        let str_expr = printer.visit_expr(&parser.parse());

        println!("{}", str_expr);
    }
}
