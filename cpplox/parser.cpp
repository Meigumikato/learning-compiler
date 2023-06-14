#include "parser.h"

#include "scanner.h"

// Parser::ParseRule Parser::parse_rules[] = {
//     [+TokenType::NUMBER] = {.prefix = &Parser::Number, .infix = {}, .precedence = Precedence::Number},
//
//     [+TokenType::MINUS] = {.prefix = &Parser::Unary,
//                            .infix = &Parser::Binary,
//                            .precedence = Precedence::Unary},
//
//     [+TokenType::PLUS] = {.prefix = {}, .infix = &Parser::Binary, .precedence = Precedence::Binary},
//     [+TokenType::STAR] = {.prefix = {}, .infix = &Parser::Binary, .precedence = Precedence::Binary},
//     [+TokenType::SLASH] = {.prefix = {}, .infix = &Parser::Binary, .precedence = Precedence::Binary},
//     [+TokenType::QUESTIONMARK] = {.prefix = {}, .infix = &Parser::Tenary, .precedence =
//     Precedence::Tenary},
// };
