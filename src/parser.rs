use std::vec;

use crate::expr::expression::*;
use crate::stmt::statement::*;

use crate::token::{Token, TokenType};
use crate::value::Value;

use thiserror::Error;

#[derive(Error, Debug)]
pub enum GrammerError {
    #[error("primary unsupported element")]
    PrimaryErr,

    #[error("group missing element")]
    GroupErr,

    #[error("unary missing element")]
    UnaryErr,

    #[error("binary missing element")]
    BinaryErr,

    #[error("ternary missing element")]
    TenaryErr,

    #[error("expression statement missing element")]
    ExprStmtErr,
}

// #[derive(Debug)]
pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new() -> Self {
        Self {
            tokens: Vec::new(),
            current: 0,
        }
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

    fn consume(&mut self, token_type: TokenType) -> Token {
        if self.peek().token_type != token_type {
            panic!("consume {:?} failed ", token_type);
        } else {
            self.advance();
            self.previous().clone()
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
    fn primary(&mut self) -> Result<Expr, GrammerError> {
        use TokenType::*;

        match self.peek().token_type {
            True => {
                self.advance();
                Ok(Expr::Literal(Value::Boolean(true)))
            }
            False => {
                self.advance();
                Ok(Expr::Literal(Value::Boolean(false)))
            }

            String | Number => {
                self.advance();
                Ok(Expr::Literal(self.previous().clone().literal))
            }
            Nil => {
                self.advance();
                Ok(Expr::Literal(Value::Nil))
            }
            LeftParen => {
                // consume LeftParen
                self.advance();

                let expr = Expr::Group(Group {
                    expr: Box::new(self.expression()?),
                });

                self.consume(RightParen);

                Ok(expr)
            }

            Identifier => {
                self.advance();

                Ok(Expr::Variable(Variable {
                    name: self.previous().clone(),
                }))
            }

            _ => Err(GrammerError::PrimaryErr),
        }
    }

    fn finish_call(&mut self, callee: Expr) -> Result<Expr, GrammerError> {
        let mut arguments = vec![];
        if !self.check(&TokenType::RightParen) {
            loop {
                if arguments.len() > 250 {
                    panic!("too many arguments")
                }

                arguments.push(self.expression()?);

                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }

        let token = self.consume(TokenType::RightParen);

        Ok(Expr::Call(Call {
            callee: Box::new(callee),
            paren: token,
            arguments,
        }))
    }

    /// call ->     primary ("(" arguments? ")") * ;
    /// arguments -> expression ( "," expression ) * ;
    fn call(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.primary()?;

        loop {
            if self.match_token(&[TokenType::LeftParen]) {
                expr = self.finish_call(expr)?
            } else {
                break;
            }
        }

        Ok(expr)
    }

    /// unary          → ( "!" | "-" ) unary | call ;
    fn unary(&mut self) -> Result<Expr, GrammerError> {
        use TokenType::*;
        if self.match_token(&[Minus, Bang]) {
            let operator = self.previous().clone();

            let next = self.unary();

            if next.is_err() {
                return Err(GrammerError::UnaryErr);
            }

            Ok(Expr::Unary(Unary {
                operator,
                expr: Box::new(next.unwrap()),
            }))
        } else {
            self.call()
        }
    }

    /// ternary       ->  unary ("?" expression ":" expression) * ;
    fn ternary(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.unary()?;

        use TokenType::*;
        while self.match_token(&[QuestionMark]) {
            let lhs = self.expression();
            if lhs.is_err() {
                return Err(GrammerError::TenaryErr);
            }

            self.consume(TokenType::Colon);

            let rhs = self.expression();
            if rhs.is_err() {
                return Err(GrammerError::TenaryErr);
            }

            expr = Expr::Ternary(Ternary {
                cond: Box::new(expr),
                lhs: Box::new(lhs.unwrap()),
                rhs: Box::new(rhs.unwrap()),
            });
        }
        Ok(expr)
    }

    /// factor         → ternary ( ( "/" | "*" ) ternary )* ;
    fn factor(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.ternary()?;

        use TokenType::*;

        while self.match_token(&[Slash, Star]) {
            let operator = self.previous().clone();
            if let Ok(right) = self.ternary() {
                expr = Expr::Binary(Binary {
                    operator,
                    lhs: Box::new(expr),
                    rhs: Box::new(right),
                })
            } else {
                return Err(GrammerError::BinaryErr);
            }
        }
        Ok(expr)
    }

    /// term           → factor ( ( "-" | "+" ) factor )* ;
    fn term(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.factor()?;

        use TokenType::*;

        while self.match_token(&[Minus, Plus]) {
            let operator = self.previous().clone();
            if let Ok(right) = self.factor() {
                expr = Expr::Binary(Binary {
                    operator,
                    lhs: Box::new(expr),
                    rhs: Box::new(right),
                })
            } else {
                return Err(GrammerError::BinaryErr);
            }
        }
        Ok(expr)
    }

    /// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    fn comparison(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.term()?;

        use TokenType::*;
        while self.match_token(&[Greater, GreaterEqual, Less, LessEqual]) {
            let operator = self.previous().clone();
            if let Ok(right) = self.term() {
                expr = Expr::Binary(Binary {
                    operator,
                    lhs: Box::new(expr),
                    rhs: Box::new(right),
                });
            } else {
                return Err(GrammerError::BinaryErr);
            }
        }
        Ok(expr)
    }

    /// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    fn equality(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.comparison()?;

        use TokenType::*;
        while self.match_token(&[BangEqual, EqualEqual]) {
            let operator = self.previous().clone();
            if let Ok(right) = self.comparison() {
                expr = Expr::Binary(Binary {
                    operator,
                    lhs: Box::new(expr),
                    rhs: Box::new(right),
                });
            } else {
                return Err(GrammerError::BinaryErr);
            }
        }
        Ok(expr)
    }

    /// logic_and -> equality ( "ans" equality ) * ;
    fn and(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.equality()?;
        while self.match_token(&[TokenType::And]) {
            let operator = self.previous().clone();
            let right = self.equality()?;

            expr = Expr::Logic(Logic {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            })
        }

        Ok(expr)
    }

    /// logic_or -> logic_and ( "or" logic_and ) * ;
    fn or(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.and()?;

        while self.match_token(&[TokenType::Or]) {
            let operator = self.previous().clone();
            let right = self.and()?;

            expr = Expr::Logic(Logic {
                operator,
                lhs: Box::new(expr),
                rhs: Box::new(right),
            })
        }

        Ok(expr)
    }

    /// assignment -> identifier "=" assignment | logic_or ;
    fn assignment(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.or()?;

        use TokenType::*;
        if self.match_token(&[Equal]) {
            let value = self.assignment()?;

            match expr {
                Expr::Variable(v) => {
                    expr = Expr::Assign(Assign {
                        name: v.name,
                        value: Box::new(value),
                    });
                }
                _ => {
                    // error handler
                    return Err(GrammerError::PrimaryErr);
                }
            }
        }

        Ok(expr)
    }

    /// comma_expr ->  assignment ("," assignment) * ;
    fn comma_expr(&mut self) -> Result<Expr, GrammerError> {
        let mut expr = self.assignment()?;

        use TokenType::*;
        while self.match_token(&[Comma]) {
            let operator = self.previous().clone();

            if let Ok(next) = self.assignment() {
                expr = Expr::Binary(Binary {
                    operator,
                    lhs: Box::new(expr),
                    rhs: Box::new(next),
                });
            } else {
                return Err(GrammerError::BinaryErr);
            }
        }
        Ok(expr)
    }

    /// expression     → comma_expr ;
    fn expression(&mut self) -> Result<Expr, GrammerError> {
        self.assignment()
    }

    // ----------------------------------------------------------
    // statement

    /// exprStmt -> expression ";" ;
    fn expression_stmt(&mut self) -> Result<Stmt, GrammerError> {
        let expr = self.expression();
        if let Ok(expr) = expr {
            self.consume(TokenType::Semicolon);

            Ok(Stmt::Expr(ExprStmt { expr }))
        } else {
            Err(GrammerError::ExprStmtErr)
        }
    }

    /// printStmt -> "print" expression ";" ;
    fn print_stmt(&mut self) -> Result<Stmt, GrammerError> {
        let expr = self.expression();
        if let Ok(expr) = expr {
            self.consume(TokenType::Semicolon);

            Ok(Stmt::Print(PrintStmt { expr }))
        } else {
            Err(GrammerError::GroupErr)
        }
    }

    /// block -> "{" declaration* "}" ;
    fn block(&mut self) -> Result<Vec<Stmt>, GrammerError> {
        let mut statements = vec![];
        while !self.check(&TokenType::RightBrace) && !self.is_at_end() {
            statements.push(self.declaration()?);
        }

        self.consume(TokenType::RightBrace);

        Ok(statements)
    }

    /// ifStmt -> "if" statement ( "else" statement) ? ;
    fn ifstmt(&mut self) -> Result<Stmt, GrammerError> {
        self.consume(TokenType::LeftParen);
        let cond = self.expression()?;
        self.consume(TokenType::RightParen);

        let then_branch = self.statement()?;
        let mut else_branch = None;

        if self.match_token(&[TokenType::Else]) {
            else_branch = Some(Box::new(self.statement()?));
        }

        Ok(Stmt::If(IfStmt {
            cond,
            then_branch: Box::new(then_branch),
            else_branch,
        }))
    }

    /// while -> "while" "(" expression ")" statement | ";" ;
    fn while_stmt(&mut self) -> Result<Stmt, GrammerError> {
        self.consume(TokenType::LeftParen);
        let cond = self.expression()?;
        self.consume(TokenType::RightParen);

        if self.match_token(&[TokenType::Semicolon]) {
            Ok(Stmt::WhileStmt(WhileStmt { cond, body: None }))
        } else {
            Ok(Stmt::WhileStmt(WhileStmt {
                cond,
                body: Some(Box::new(self.statement()?)),
            }))
        }
    }

    /// forStmt -> "for" "(" varDecl | expression_stmt | ";" expression? ";" expression? ")" statement
    fn for_stmt(&mut self) -> Result<Stmt, GrammerError> {
        self.consume(TokenType::LeftParen);

        let initializer;
        if self.match_token(&[TokenType::Var]) {
            initializer = Some(self.var_declaration()?);
        } else if self.match_token(&[TokenType::Semicolon]) {
            initializer = None
        } else {
            initializer = Some(self.expression_stmt()?);
        }

        let mut condition = None;
        if let Ok(cond) = self.expression() {
            condition = Some(cond);
        }

        self.consume(TokenType::Semicolon);

        let mut increment = None;
        if let Ok(inc) = self.expression() {
            increment = Some(inc);
        }

        self.consume(TokenType::RightParen);

        let mut body = None;
        if !self.match_token(&[TokenType::Semicolon]) {
            if let Ok(b) = self.statement() {
                body = Some(b);
            }
        }

        let mut desugaring = None;

        if let Some(b) = body {
            desugaring = Some(Stmt::Block(BlockStmt {
                statements: vec![b],
            }))
        }

        if let Some(inc) = increment {
            desugaring = Some(Stmt::Block(BlockStmt {
                statements: if let Some(de) = desugaring {
                    vec![de, Stmt::Expr(ExprStmt { expr: inc })]
                } else {
                    vec![Stmt::Expr(ExprStmt { expr: inc })]
                },
            }))
        }

        desugaring = Some(Stmt::WhileStmt(WhileStmt {
            cond: if let Some(cond) = condition {
                cond
            } else {
                Expr::Literal(Value::Boolean(true))
            },
            body: desugaring.map(Box::new),
        }));

        if let Some(init) = initializer {
            desugaring = Some(Stmt::Block(BlockStmt {
                statements: vec![init, desugaring.unwrap()],
            }))
        }

        Ok(desugaring.unwrap())
    }

    /// statement -> exprStmt | printStmt | block | ifStmt | whileStmt | forStmt ;
    fn statement(&mut self) -> Result<Stmt, GrammerError> {
        match self.peek().token_type {
            TokenType::If => {
                self.advance();
                self.ifstmt()
            }
            TokenType::While => {
                self.advance();
                self.while_stmt()
            }
            TokenType::For => {
                self.advance();
                self.for_stmt()
            }
            TokenType::Print => {
                self.advance();
                self.print_stmt()
            }
            TokenType::LeftBrace => {
                self.advance();
                Ok(Stmt::Block(BlockStmt {
                    statements: self.block()?,
                }))
            }
            _ => self.expression_stmt(),
        }
    }

    /// varDecl ->  "var" identifier ("=" expression )? ";" ;
    fn var_declaration(&mut self) -> Result<Stmt, GrammerError> {
        let token = self.consume(TokenType::Identifier);

        let mut initializer = Expr::Nil;
        if self.match_token(&[TokenType::Equal]) {
            initializer = self.expression()?;
        }

        self.consume(TokenType::Semicolon);

        Ok(Stmt::Var(VarStmt { token, initializer }))
    }

    /// fun Decl -> "fun" "(" parameters? ")" block ;
    /// parameters -> identifier ("," identifier) * ;
    fn fun_declaration(&mut self, _: &str) -> Result<Stmt, GrammerError> {
        let name = self.consume(TokenType::Identifier);

        self.consume(TokenType::LeftParen);

        let mut parameters = vec![];
        if !self.check(&TokenType::RightParen) {
            loop {
                if parameters.len() > 250 {
                    panic!("fun_declaration has too many parameters")
                }

                let identifier = self.consume(TokenType::Identifier);
                parameters.push(identifier);

                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }

        self.consume(TokenType::RightParen);

        // function body
        self.consume(TokenType::LeftBrace);
        let body = self.block()?;

        Ok(Stmt::FunStmt(FunStmt {
            name,
            parameters,
            block: body,
        }))
    }

    // declaration -> varDecl | funDecl | statement ;
    fn declaration(&mut self) -> Result<Stmt, GrammerError> {
        if self.match_token(&[TokenType::Var]) {
            self.var_declaration()
        } else if self.match_token(&[TokenType::Fun]) {
            self.fun_declaration("function")
        } else {
            self.statement()
        }
    }

    /// program -> statement* EOF ;
    pub fn parse(&mut self, tokens: Vec<Token>) -> Vec<Stmt> {
        self.current = 0;
        self.tokens = tokens;

        let mut statements = vec![];

        while !self.is_at_end() {
            match self.declaration() {
                Ok(stmt) => statements.push(stmt),
                Err(e) => {
                    // panic mode, synchronize
                    panic!("parse error {}", e)
                }
            }
        }

        statements
    }
}
