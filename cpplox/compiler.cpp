#include "compiler.h"

#include <cstdio>
#include <cstdlib>

#include "chunk.h"
#include "common.h"
#include "scanner.h"

void Compiler::Advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanner.ScanToken();

    if (parser.current.type == TokenType::ERROR) break;
  }

  ErrorAtCurrent(parser.current.start);
}

void Compiler::ErrorAt(Token* token, const char* message) {
  if (parser.panic_mode) return;

  parser.panic_mode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::TEOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TokenType::ERROR) {
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);

  parser.had_error = true;
}

void Compiler::Consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    Advance();
    return;
  }

  ErrorAtCurrent(message);
}

bool Compiler::Compile(const char* source, Chunk* chunk) {
  chunk = chunk;

  Advance();
  Expression();
  Consume(TokenType::TEOF, "Expect end of expression.");

  FinishCompile();

  return !parser.had_error;
}

void Compiler::EmitByte(uint8_t byte) {
  chunk->Write(byte, parser.previous.line);
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
  int constant = chunk->AddConstant(value);
  if (constant > UINT8_MAX) {
    // TODO: OP_CONSTANT_LONG
    Error("Too may constants in one chunk.");
    abort();
  }

  return (uint8_t)constant;
}

void Compiler::Number() {
  double value = strtod(parser.previous.start, nullptr);
  EmitConstant(value);
}

void Compiler::Grouping() {
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::Unary() {
  auto op_type = parser.previous.type;

  Expression();

  switch (op_type) {
    case TokenType::MINUS:
      EmitByte(OP_NEGATE);
      break;
    default:
      return;
  }
}
