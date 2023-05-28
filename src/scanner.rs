use crate::token::{Literal, Token, TokenType};
use std::collections::HashMap;

fn report(line: usize, where_error: String, message: &str) {
    println!("[line:{}] Error{}:{}", line, where_error, message)
}

fn error(line: usize, message: &str) {
    report(line, "".to_string(), message)
}

pub struct Scanner {
    source: String,
    tokens: Vec<Token>,

    start: usize,
    current: usize,
    line: usize,

    key_words: HashMap<&'static str, TokenType>,
}

impl Scanner {
    pub fn new(source: String) -> Self {
        let mut scanner = Self {
            source,
            tokens: Vec::new(),
            start: 0,
            current: 0,
            line: 1,
            key_words: HashMap::new(),
        };

        scanner.key_words.insert("and", TokenType::And);
        scanner.key_words.insert("class", TokenType::Class);
        scanner.key_words.insert("else", TokenType::Else);
        scanner.key_words.insert("false", TokenType::False);
        scanner.key_words.insert("for", TokenType::For);
        scanner.key_words.insert("fun", TokenType::Fun);
        scanner.key_words.insert("if", TokenType::If);
        scanner.key_words.insert("nil", TokenType::Nil);
        scanner.key_words.insert("or", TokenType::Or);
        scanner.key_words.insert("print", TokenType::Print);
        scanner.key_words.insert("return", TokenType::Return);
        scanner.key_words.insert("super", TokenType::Super);
        scanner.key_words.insert("this", TokenType::This);
        scanner.key_words.insert("true", TokenType::True);
        scanner.key_words.insert("var", TokenType::Var);
        scanner.key_words.insert("while", TokenType::While);

        scanner
    }

    fn substring(&self, start: usize, len: usize) -> String {
        self.source.chars().skip(start).take(len).collect()
    }

    fn is_at_end(&self) -> bool {
        self.current >= self.source.len()
    }

    pub fn scan_tokens(mut self) -> Vec<Token> {
        while !self.is_at_end() {
            self.start = self.current;
            self.scan_token();
        }

        self.tokens.push(Token::new(
            TokenType::Eof,
            Literal::None,
            "".to_string(),
            self.line,
        ));

        self.tokens
    }

    fn advance(&mut self) -> char {
        let temp = self.current;
        self.current += 1;
        self.source.chars().nth(temp).unwrap()
    }

    fn add_token(&mut self, token_type: TokenType) {
        self.add_token_with_literal(token_type, Literal::None)
    }

    fn add_token_with_literal(&mut self, token_type: TokenType, literal: Literal) {
        self.tokens.push(Token::new(
            token_type,
            literal,
            self.substring(self.start, self.current - self.start),
            self.line,
        ))
    }

    //
    fn match_char(&mut self, expected: char) -> bool {
        if self.is_at_end() {
            return false;
        }

        if self
            .source
            .chars()
            .nth(self.current)
            .expect("get current character failed")
            != expected
        {
            return false;
        }

        self.current += 1;
        true
    }

    fn peek(&self) -> char {
        if self.current >= self.source.len() {
            '\0'
        } else {
            self.source
                .chars()
                .nth(self.current)
                .expect("peek current character failed")
        }
    }

    fn peek_next(&self) -> char {
        if self.current + 1 >= self.source.len() {
            '\0'
        } else {
            self.source
                .chars()
                .nth(self.current + 1)
                .expect("peek current character failed")
        }
    }

    fn string(&mut self) {
        while self.peek() != '"' && !self.is_at_end() {
            if self.peek() == '\n' {
                self.line += 1;
            }
            self.advance();
        }

        if self.is_at_end() {
            error(self.line, "Unterminated string.")
        }

        // consume closeing "
        self.advance();
        let literal =
            Literal::String(self.substring(self.start + 1, self.current - self.start - 1));

        self.add_token_with_literal(TokenType::String, literal)
    }

    fn number(&mut self) {
        while self.peek().is_ascii_digit() {
            self.advance();
        }

        if self.peek() == '.' && self.peek_next().is_ascii_digit() {
            // consume dot
            self.advance();

            while self.peek().is_ascii_digit() {
                self.advance();
            }
        }

        let literal = Literal::Double(
            self.substring(self.start, self.current - self.start)
                .parse::<f64>()
                .expect("parse to f64 failed"),
        );

        self.add_token_with_literal(TokenType::Number, literal);
    }

    fn identifier(&mut self) {
        while self
            .source
            .chars()
            .nth(self.current)
            .expect("identifier get alpnum")
            .is_digit(10)
        {
            self.advance();
        }

        let identifier = self.substring(self.start, self.current - self.start);

        if let Some(reserve_type) = self.key_words.get(identifier.as_str()) {
            self.add_token(reserve_type.clone());
        } else {
            self.add_token(TokenType::Identifier);
        }
    }

    fn scan_token(&mut self) {
        loop {
            let c = self.advance();
            match c {
                '(' => self.add_token(TokenType::LeftParen),
                ')' => self.add_token(TokenType::RightParen),
                '{' => self.add_token(TokenType::LeftBrace),
                '}' => self.add_token(TokenType::RightBrace),
                ',' => self.add_token(TokenType::Comma),
                '.' => self.add_token(TokenType::Dot),
                '+' => self.add_token(TokenType::Plus),
                '-' => self.add_token(TokenType::Minus),
                ';' => self.add_token(TokenType::Semicolon),
                '*' => self.add_token(TokenType::Star),

                '!' if self.match_char('=') => self.add_token(TokenType::BangEqual),
                '!' => self.add_token(TokenType::Bang),

                '=' if self.match_char('=') => self.add_token(TokenType::EqualEqual),
                '=' => self.add_token(TokenType::Equal),

                '<' if self.match_char('=') => self.add_token(TokenType::LessEqual),
                '<' => self.add_token(TokenType::Less),

                '>' if self.match_char('=') => self.add_token(TokenType::GreaterEqual),
                '>' => self.add_token(TokenType::Greater),

                // comment consume
                '/' if self.match_char('/') => {
                    while !self.is_at_end() && self.peek() != '\n' {
                        self.advance();
                    }
                }
                '/' => self.add_token(TokenType::Slash),

                ' ' | '\r' | '\t' => {
                    // skip
                }

                '\n' => self.line += 1,

                '"' => self.string(),

                '0'..='9' => self.number(),

                'a'..='z' | 'A'..='Z' | '_' => self.identifier(),

                _ => {
                    error(self.line, "Unexpected character");
                    break;
                }
            }
        }
    }
}
