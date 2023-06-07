#pragma once

#include "chunk.h"
#include "scanner.h"
#include "vm.h"

struct Compiler {
  Scanner scanner;

  struct Parser {
    Token current;
    Token previous;
    bool had_error{};
    bool panic_mode{};
  };

  Parser parser;

  Chunk* chunk;

  void ErrorAtCurrent(const char* message) {
    ErrorAt(&parser.current, message);
  }

  void ErrorAt(Token* token, const char* message);

  void Error(const char* message) { ErrorAt(&parser.current, message); }

  void Consume(TokenType type, const char* message);

  bool Compile(const char* source, Chunk* chunk);

  void FinishCompile() { EmitReturn(); }

  void Advance();

  void EmitByte(uint8_t byte);

  void EmitBytes(uint8_t byte1, uint8_t byte2);

  void EmitReturn();

  void EmitConstant(Value value);
  uint8_t MakeConstant(Value value);

  void Expression();

  void Number();

  void Grouping();

  void Unary();
};

bool compile(const char* source, Chunk* chunk);
