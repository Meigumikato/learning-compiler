#pragma once

#include <cstdint>
#include <vector>

#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "vm.h"

class Compiler {
 public:
  Compiler(const char* source) : source_(source), scanner_(source) {}

  ObjFunction* Compile();

  // Chunk* GetChunk() { return &chunk_; }

 private:
  struct Local {
    Token name;
    int depth{};
  };

  struct Parser {
    Token current{};
    Token previous{};
    bool had_error{};
    bool panic_mode{};
  };

  enum class FunctionType {
    FUNCTION,
    SCRIPT,
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

  using ParseFn = void (Compiler::*)(bool);
  struct ParseRule {
    friend Compiler;
    ParseFn prefix{};
    ParseFn infix{};
    Compiler::Precedence precedence{};
  };

  struct FuncScope {
    FuncScope* enclosing{};
    ObjFunction* function{};
    FunctionType func_type;
    std::vector<Local> locals;

    ~FuncScope() {
      enclosing = nullptr;
      delete function;
    }
  };

  FuncScope* current_;

  static const ParseRule rules[(int)TokenType::SENTINAL];

  const char* source_;

  Scanner scanner_;

  Parser parser_;

  int scope_depth_{};

  static const ParseRule* GetRule(TokenType type);

  void ErrorAtCurrent(const char* message) {
    ErrorAt(&parser_.current, message);
  }

  bool Match(TokenType type);
  bool Check(TokenType type);

  void ErrorAt(Token* token, const char* message);

  void Error(const char* message) { ErrorAt(&parser_.current, message); }

  void Consume(TokenType type, const char* message);

  ObjFunction* FinishCompile();

  void Advance();

  void EmitByte(uint8_t byte);

  void EmitBytes(uint8_t byte1, uint8_t byte2);

  void EmitReturn();

  void EmitConstant(Value value);

  uint8_t MakeConstant(Value value);

  uint16_t EmitJump(uint8_t Instruction);

  void EmitLoop(int loop_start);

  void PatchJump(int offset);

  void BeginScope();

  void EndScope();

  bool IdentifierEqual(Token* a, Token* b);

  uint8_t IdentifierConstant(Token* offset);

  void ParsePrecedence(Precedence precedence);

  void AddLocal(Token name);

  void DeclareVariable();

  uint8_t ParseVariable(const char* msg);

  void DefineVariable(uint8_t global);

  void MarkInitialized();

  int ResolveLocal(Token* name);

  void Synchronize();

  void Number(bool can_assign);

  void Grouping(bool can_assign);

  void Unary(bool can_assign);

  void Binary(bool can_assign);

  void Ternary(bool can_assign);

  void Literal(bool can_assign);

  void String(bool can_assign);

  void Variable(bool can_assign);

  void And(bool can_assign);

  void Or(bool can_assign);

  uint8_t ArgumentList();

  void Call(bool can_assign);

  void NamedVariable(Token token, bool can_assign);

  void Expression();

  void Statement();

  void PrintStatement();

  void BlockStatement();

  void IfStatement();

  void WhileStatement();

  void ReturnStatement();

  void ForStatement();

  void Function(FunctionType type);

  void ExpressionStatement();

  void VarDeclaration();

  void FunDeclaration();

  void Declaration();
};
