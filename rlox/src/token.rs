use crate::value::LoxValue;

#[derive(Debug, Clone, PartialEq)]
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
    Colon,
    Slash,
    Star,
    QuestionMark,
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
    Break,
    Super,
    This,
    True,
    Var,
    While,

    Eof,
}

// #[derive(Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub literal: LoxValue,
    pub lexeme: String,
    pub line: usize,
}

impl Clone for Token {
    fn clone(&self) -> Self {
        Self {
            token_type: self.token_type.clone(),
            literal: LoxValue(self.literal.0.clone()),
            lexeme: self.lexeme.clone(),
            line: self.line.clone(),
        }
    }
}

impl Token {
    pub fn new(token_type: TokenType, literal: LoxValue, lexeme: String, line: usize) -> Self {
        Self {
            token_type,
            literal,
            lexeme,
            line,
        }
    }
}
