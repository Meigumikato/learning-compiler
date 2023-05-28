use std::{env, fs, io};

mod scanner;
mod token;
use crate::scanner::Scanner;

fn run(script: &String) {
    let scanner = Scanner::new(script.clone());
    scanner.scan_tokens();
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

    if args.len() > 1 {
        println!("Usage rlox [script]");
    } else if args.len() == 1 {
        run_file(args.first().unwrap());
    } else {
        run_prompt();
    }
}
