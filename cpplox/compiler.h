#pragma once

#include <vector>

#include "chunk.h"
#include "scanner.h"
#include "vm.h"

class Compiler {
 public:
  Compiler(const char* source) : source_(source), scanner_(source) {}

  bool Compile();

  Chunk* GetChunk() { return &chunk_; }

 private:
  struct Parser {
    Token current{};
    Token previous{};
    bool had_error{};
    bool panic_mode{};
  };

  enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_TERNARY,
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
  };

  using ParseFn = void (Compiler::*)();
  struct ParseRule {
    friend Compiler;
    ParseFn prefix{};
    ParseFn infix{};
    Compiler::Precedence precedence{};
  };

  static const ParseRule rules[(int)TokenType::SENTINAL];

  const char* source_;

  Scanner scanner_;

  Parser parser_;

  Chunk chunk_;

  static const ParseRule* GetRule(TokenType type);

  void ErrorAtCurrent(const char* message) {
    ErrorAt(&parser_.current, message);
  }

  void ErrorAt(Token* token, const char* message);

  void Error(const char* message) { ErrorAt(&parser_.current, message); }

  void Consume(TokenType type, const char* message);

  void FinishCompile();

  void Advance();

  void EmitByte(uint8_t byte);

  void EmitBytes(uint8_t byte1, uint8_t byte2);

  void EmitReturn();

  void EmitConstant(Value value);

  uint8_t MakeConstant(Value value);

  void ParsePrecedence(Precedence precedence);

  void Expression();

  void Number();

  void Grouping();

  void Unary();

  void Binary();

  void Ternary();
};

bool compile(const char* source, Chunk* chunk);
