#include "compiler.h"

#include <cstdio>
#include <cstdlib>

#include "chunk.h"
#include "common.h"
#include "scanner.h"

const Compiler::ParseRule Compiler::rules[(int)TokenType::SENTINAL] = {
    // [(int)TokenType::LEFT_PAREN] =
    {.prefix = &Compiler::Grouping,
     .infix = nullptr,
     .precedence = Compiler::PREC_NONE},
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
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::BANG_EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::EQUAL_EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::GREATER] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::GREATER_EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::LESS] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::LESS_EQUAL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::IDENTIFIER] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::STRING] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::NUMBER] =
    {.prefix = &Compiler::Number, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::AND] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::CLASS] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::ELSE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FALSE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FOR] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::FUN] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::IF] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::NIL] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::OR] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::PRINT] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::RETURN] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::SUPER] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::THIS] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
    // [(int)TokenType::TRUE] =
    {.prefix = nullptr, .infix = nullptr, .precedence = PREC_NONE},
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

void Compiler::Advance() {
  parser_.previous = parser_.current;

  for (;;) {
    parser_.current = scanner_.ScanToken();

    if (parser_.current.type != TokenType::ERROR) break;
    ErrorAtCurrent(parser_.current.start);
  }
}

void Compiler::ErrorAt(Token* token, const char* message) {
  if (parser_.panic_mode) return;

  parser_.panic_mode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::TEOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TokenType::ERROR) {
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);

  parser_.had_error = true;
}

void Compiler::Consume(TokenType type, const char* message) {
  if (parser_.current.type == type) {
    Advance();
    return;
  }

  ErrorAtCurrent(message);
}

void Compiler::FinishCompile() {
  EmitReturn();

#ifdef DEBUG_PRINT_CODE
  if (!parser_.had_error) {
    chunk_.Disassemble("code");
  }
#endif
}

bool Compiler::Compile() {
  Advance();
  Expression();
  Consume(TokenType::TEOF, "Expect end of expression.");

  FinishCompile();

  return !parser_.had_error;
}

void Compiler::EmitByte(uint8_t byte) {
  chunk_.Write(byte, parser_.previous.line);
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
  EmitByte(byte1);
  EmitByte(byte2);
}

void Compiler::EmitReturn() { EmitByte(OP_RETURN); }

void Compiler::EmitConstant(Value value) {
  EmitBytes(OP_CONSTANT, MakeConstant(value));
}

uint8_t Compiler::MakeConstant(Value value) {
  int constant = chunk_.AddConstant(value);
  if (constant > UINT8_MAX) {
    // TODO: OP_CONSTANT_LONG
    Error("Too may constants in one chunk.");
    abort();
  }

  return (uint8_t)constant;
}

void Compiler::ParsePrecedence(Precedence precedence) {
  Advance();
  auto prefix_rule = GetRule(parser_.previous.type)->prefix;
  if (prefix_rule == nullptr) {
    Error("Expect Expression.");
    return;
  }

  (this->*prefix_rule)();

  while (precedence <= GetRule(parser_.current.type)->precedence) {
    Advance();
    auto infix_rule = GetRule(parser_.previous.type)->infix;
    (this->*infix_rule)();
  }
}

void Compiler::Expression() { ParsePrecedence(PREC_ASSIGNMENT); }

void Compiler::Number() {
  double value = strtod(parser_.previous.start, nullptr);
  EmitConstant(NUMBER_VAL(value));
}

void Compiler::Grouping() {
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::Unary() {
  auto op_type = parser_.previous.type;

  // compile the operand
  ParsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (op_type) {
    case TokenType::MINUS:
      EmitByte(OP_NEGATE);
      break;
    default:
      return;
  }
}

void Compiler::Binary() {
  auto operator_type = parser_.previous.type;
  auto parse_rule = GetRule(operator_type);
  //
  ParsePrecedence((Precedence)((int)(parse_rule->precedence) + 1));
  //
  switch (operator_type) {
    case TokenType::PLUS:
      EmitByte(OP_ADD);
      break;
    case TokenType::MINUS:
      EmitByte(OP_SUBTRACT);
      break;
    case TokenType::STAR:
      EmitByte(OP_MULTIPLY);
      break;
    case TokenType::SLASH:
      EmitByte(OP_DIVIDE);
      break;
    default:
      return;  // unreachable
  }
}

void Compiler::Ternary() {}
