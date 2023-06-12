#include "compiler.h"

const Compiler::ParseRule Compiler::rules[(int)TokenType::SENTINAL] = {
    // [(int)TokenType::LEFT_PAREN] =
    {.prefix = &Compiler::Grouping,
     .infix = &Compiler::Call,
     .precedence = Compiler::PREC_CALL},
    // [(int)TokenType::RIGHT_PAREN] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::LEFT_BRACE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::RIGHT_BRACE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::COMMA] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::COLON] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::DOT] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::MINUS] =
    {.prefix = &Compiler::Unary,
     .infix = &Compiler::Binary,
     .precedence = PREC_TERM},
    // [(int)TokenType::PLUS] =
    {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_TERM},
    // [(int)TokenType::SEMICOLON] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::SLASH] =
    {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_FACTOR},
    // [(int)TokenType::STAR] =
    {.prefix = nullptr, .infix = &Compiler::Binary, .precedence = PREC_FACTOR},
    // [TokenType::QUESTIONMARK]
    {.prefix = nullptr,
     .infix = &Compiler::Ternary,
     .precedence = PREC_TERNARY},

    // [(int)TokenType::BANG] =
    {.prefix = &Compiler::Unary, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::BANG_EQUAL] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_EQUALITY},
    // [(int)TokenType::EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::EQUAL_EQUAL] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_EQUALITY},
    // [(int)TokenType::GREATER] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_COMPARISON},
    // [(int)TokenType::GREATER_EQUAL] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_COMPARISON},
    // [(int)TokenType::LESS] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_COMPARISON},
    // [(int)TokenType::LESS_EQUAL] =
    {.prefix = nullptr,
     .infix = &Compiler::Binary,
     .precedence = PREC_COMPARISON},
    // [(int)TokenType::IDENTIFIER] =
    {.prefix = &Compiler::Variable, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::STRING] =
    {.prefix = &Compiler::String, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::NUMBER] =
    {.prefix = &Compiler::Number, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::AND] =
    {.prefix = nullptr, .infix = &Compiler::And, .precedence = PREC_NONE},
    // [(int)TokenType::CLASS] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::ELSE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FALSE] =
    {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FOR] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FUN] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::IF] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::NIL] =
    {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::OR] =
    {.prefix = nullptr, .infix = &Compiler::Or, .precedence = PREC_NONE},
    // [(int)TokenType::PRINT] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::RETURN] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::SUPER] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::THIS] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::TRUE] =
    {.prefix = &Compiler::Literal, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::VAR] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::WHILE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::ERROR] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::TEOF] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
};

const Compiler::ParseRule* Compiler::GetRule(TokenType type) {
  return &rules[(int)type];
}
