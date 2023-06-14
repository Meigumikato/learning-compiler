#pragma once

#include <cassert>
#include <functional>

#include "compiler.h"
#include "scanner.h"

// enum class Precedence {
//   None,
//   Tenary,
//   Binary,
//   Unary,
//   Number,
// };
//
// class Parser {
//  public:
//   Parser(const char* source) : scanner(source) {}
//
//   void Parse() {
//     Advance();  // prime plumb
//     Expression();
//   }
//
//  private:
//   Scanner scanner;
//
//   using ParseFunctor = std::function<void(Parser*)>;
//
//   struct Helper {
//     Token previous;
//     Token current;
//   };
//
//   struct ParseRule {
//     ParseFunctor prefix;
//     ParseFunctor infix;
//     Precedence precedence;
//   };
//
//   Helper token_pair;
//
//   void Advance() {
//     token_pair.previous = token_pair.current;
//     token_pair.current = scanner.ScanToken();
//   }
//
//   void Consume(TokenType type, const char* err_msg) {
//     assert(type == token_pair.previous.type);
//     Advance();
//   }
//
//   static ParseRule parse_rules[];
//
//   ParseRule GetRule(Token token) { return parse_rules[+token.type]; }
//
//   void ParsePrecedence(Precedence precedence) {
//     Advance();
//     auto rule = GetRule(token_pair.previous);
//
//     // prefix parser
//     rule.prefix(this);
//
//     while (precedence <= GetRule(token_pair.current).precedence) {
//       auto rule = GetRule(token_pair.current);
//       Advance();
//       rule.infix(this);
//     }
//   }
//
//   void Number() {
//     assert(token_pair.previous.type == TokenType::Number);
//     double d = strtod(token_pair.previous.start, nullptr);
//   }
//
//   void Unary() {
//     auto op_type = token_pair.previous.type;
//
//     ParsePrecedence(Precedence::Unary);
//
//     switch (op_type) {
//       case TokenType::Minus:
//         break;
//       case TokenType::Bang:
//         break;
//       default:
//         assert(0);
//     }
//   }
//
//   void Binary() {
//     auto op = token_pair.previous.type;
//     // left associative
//     ParsePrecedence((Precedence)(+Precedence::Binary + 1));
//
//     switch (op) {
//       case TokenType::Plus:
//         break;
//       case TokenType::Minus:
//         break;
//       case TokenType::Star:
//         break;
//       case TokenType::Slash:
//         break;
//       default:
//         assert(0);
//     }
//   }
//
//   void Tenary() {
//     ParsePrecedence(Precedence::Tenary);
//     Consume(TokenType::Colon, "Expect colon in Tenary Expression");
//     ParsePrecedence(Precedence::Tenary);
//   }
//
//   void Expression() {
//     // Advance();
//     ParsePrecedence(Precedence::None);
//   }
// };
