#[derive(Clone)]
pub enum TokenType {
    // Single-character tokens.
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Dot,
    Minus,
    Plus,
    Semicolon,
    Slash,
    Star,
    // One or two character tokens.
    Bang,
    BangEqual,
    Equal,
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    // Literals.
    Identifier,
    String,
    Number,
    // Keywords.
    And,
    Class,
    Else,
    False,
    Fun,
    For,
    If,
    Nil,
    Or,
    Print,
    Return,
    Super,
    This,
    True,
    Var,
    While,

    Eof,
}

pub enum Literal {
    None,
    String(String),
    Double(f64),
    // Vec(Vec<i32>),
}

// #[derive(Debug)]
pub struct Token {
    token_type: TokenType,
    literal: Literal,
    lexeme: String,
    line: usize,
}

impl Token {
    pub fn new(token_type: TokenType, literal: Literal, lexeme: String, line: usize) -> Self {
        Self {
            token_type,
            literal,
            lexeme,
            line,
        }
    }
}
