use crate::expr::{Binary, Expr, Group, Unary};
use crate::token::{Literal, Token, TokenType};

#[derive(Debug)]
pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, current: 0 }
    }

    fn is_at_end(&self) -> bool {
        self.peek().token_type == TokenType::Eof
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.current]
    }

    fn previous(&self) -> &Token {
        &self.tokens[self.current - 1]
    }

    fn advance(&mut self) -> &Token {
        if !self.is_at_end() {
            self.current += 1
        }
        self.previous()
    }

    fn consume(&mut self, token_type: TokenType) -> bool {
        if self.peek().token_type != token_type {
            false
        } else {
            self.advance();
            true
        }
    }

    fn check(&self, token_type: &TokenType) -> bool {
        if self.is_at_end() {
            false
        } else {
            self.peek().token_type.eq(token_type)
        }
    }

    fn match_token(&mut self, token_types: &[TokenType]) -> bool {
        for token_type in token_types {
            if self.check(token_type) {
                self.advance();
                return true;
            }
        }

        false
    }

    /// primary        → NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;
    fn primary(&mut self) -> Expr {
        use TokenType::*;

        match self.peek().token_type {
            True => {
                self.advance();
                Expr::Literal(Literal::Boolean(true))
            }
            False => {
                self.advance();
                Expr::Literal(Literal::Boolean(false))
            }

            String | Number => {
                self.advance();
                Expr::Literal(self.previous().clone().literal)
            }
            Nil => {
                self.advance();
                Expr::Literal(Literal::Nil)
            }
            LeftParen => {
                // consume LeftParen
                self.advance();

                let expr = self.expression();

                if !self.consume(RightParen) {
                    panic!("RightParen missing")
                }

                Expr::Group(Group {
                    expr: Box::new(expr),
                })
            }
            _ => {
                panic!("unexpected token_type {:?}", self.peek().token_type)
            }
        }
    }

    /// unary          → ( "!" | "-" ) unary | primary ;
    fn unary(&mut self) -> Expr {
        use TokenType::*;
        if self.match_token(&[Minus, Bang]) {
            let operator = self.previous().clone();
            return Expr::Unary(Unary {
                operator,
                expr: Box::new(self.unary()),
            });
        }

        self.primary()
    }

    /// factor         → unary ( ( "/" | "*" ) unary )* ;
    fn factor(&mut self) -> Expr {
        let mut expr = self.unary();

        use TokenType::*;

        while self.match_token(&[Slash, Star]) {
            let operator = self.previous().clone();
            let right = self.unary();
            expr = Expr::Binary(Binary {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            })
        }

        expr
    }

    /// term           → factor ( ( "-" | "+" ) factor )* ;
    fn term(&mut self) -> Expr {
        let mut expr = self.factor();

        use TokenType::*;

        while self.match_token(&[Minus, Plus]) {
            let operator = self.previous().clone();
            let right = self.factor();
            expr = Expr::Binary(Binary {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            })
        }

        expr
    }

    /// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    fn comparison(&mut self) -> Expr {
        let mut expr = self.term();

        use TokenType::*;
        while self.match_token(&[Greater, GreaterEqual, Less, LessEqual]) {
            let operator = self.previous().clone();
            let right = self.term();
            expr = Expr::Binary(Binary {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            });
        }

        expr
    }

    /// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    fn equality(&mut self) -> Expr {
        let mut expr = self.comparison();

        use TokenType::*;
        while self.match_token(&[BangEqual, EqualEqual]) {
            let operator = self.previous().clone();
            let right = self.comparison();
            expr = Expr::Binary(Binary {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            });
        }

        expr
    }

    /// ternary       -> ( ( ( ternary "?" ) * ) | ( ( ternary ":") * ) )  * expression ;
    fn ternary(&mut self) -> Expr {
        let expr = self.ternary();

        use TokenType::*;
        while self.match_token(&[QuestionMark]) {
            let expr = self.ternary();
        }

        while self.match_token(&[Colon]) {
            let expr = self.ternary();
        }

        expr
    }

    /// expression     → equality ;
    fn expression(&mut self) -> Expr {
        self.equality()
    }

    pub fn parse(&mut self) -> Expr {
        self.expression()
    }
}
