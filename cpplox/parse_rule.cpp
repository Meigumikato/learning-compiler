#include "compiler.h"
#include "scanner.h"
#include "common.h"

const Compiler::ParseRule Compiler::rules[] = {
    // [+TokenType::LeftParen] = 
  {.prefix = &Compiler::Grouping, .infix = &Compiler::Call, .precedence = PREC_CALL},
    
    // [+TokenType::RightParen] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::LeftBrace] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::RightBrace] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Comma] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Colon] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Dot] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Minus] =
  {.prefix = &Compiler::Unary, .infix = &Compiler::Binary, .precedence = PREC_TERM},
    // [+TokenType::Plus] =
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_TERM},
    // [+TokenType::Semicolon] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Slash] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_FACTOR},
    // [+TokenType::Star] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_FACTOR},
    // [+TokenType::QuestionMark] = 
  {.prefix = nullptr, .infix = &Compiler::Ternary, .precedence = PREC_TERNARY},
    // [+TokenType::Bang] = 
  {.prefix = &Compiler::Unary, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::BangEqual] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_EQUALITY},
    // [+TokenType::Equal] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::EqualEqual] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_EQUALITY},
    // [+TokenType::Greater] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_COMPARISON},
    // [+TokenType::GreaterEqual] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_COMPARISON},
    // [+TokenType::Less] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_COMPARISON},
    // [+TokenType::LessEqual] = 
  {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_COMPARISON},
    // [+TokenType::Identifier] = 
  {.prefix = &Compiler::Variable, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::String] = 
  {.prefix = &Compiler::String, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Number] = 
  {.prefix = &Compiler::Number, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::And] = 
  {.prefix = nullptr, .infix = &Compiler::And, .precedence = PREC_NONE},
    // [+TokenType::Class] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Else] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::False] = 
  {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::For] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Fun] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::If] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Nil] = 
  {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Or] = 
  {.prefix = nullptr, .infix = &Compiler::Or, .precedence = PREC_NONE},
    // [+TokenType::Print] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Return] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Super] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::This] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::True] = 
  {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Var] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::While] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Error] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [+TokenType::Eof] = 
  {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
};

const Compiler::ParseRule* Compiler::GetRule(TokenType type) { return &rules[(int)type]; }
