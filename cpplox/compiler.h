#pragma once

#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "vm.h"

class Compiler {
 public:
  Compiler(const char* source, VM* vm) : vm_(vm), source_(source), scanner_(source) {}

  std::unique_ptr<Function> Compile();

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
    PREC_TERNARY,     // ?
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

  struct Loop {
    int start;
    int stop;
    std::vector<uint8_t> breaks;
    std::vector<uint8_t> continues;
  };

  struct FuncScope {
    // ref
    FuncScope* enclosing{};

    std::unique_ptr<Function> function{};

    FunctionType func_type;

    int scope_depth_{};
    int loop_depth_{};

    std::vector<Local> locals;
    std::vector<Loop> loops;

    FuncScope(FunctionType type) : function(std::make_unique<Function>()), func_type(type) {
      Local local;
      local.name.start = "";
      local.name.length = 0;
      local.depth = 0;

      locals.push_back(local);
    }
    ~FuncScope() { enclosing = nullptr; }
  };

  VM* vm_;

  FuncScope* current_;

  static const ParseRule rules[];

  const char* source_;

  Scanner scanner_;

  Parser parser_;

  static const ParseRule* GetRule(TokenType type);

  void ErrorAtCurrent(const char* message) { ErrorAt(&parser_.current, message); }

  bool Match(TokenType type);
  bool Check(TokenType type);

  void ErrorAt(Token* token, const char* message);

  void Error(const char* message) { ErrorAt(&parser_.current, message); }

  void Consume(TokenType type, const char* message);

  std::unique_ptr<Function> FinishCompile();

  void Advance();

  void EmitByte(uint8_t byte);

  void EmitBytes(uint8_t byte1, uint8_t byte2);

  void EmitReturn();

  void EmitConstant(Value value);

  uint8_t MakeConstant(Value value);

  uint16_t EmitJump(uint8_t Instruction);

  uint16_t EmitLoop(int loop_start);

  void PatchJump(int offset);

  void PatchJumpWithOffset(int offset, uint16_t dest);

  void BeginScope();

  void EndScope();

  void BeginLoop();

  void EndLoop();

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

  void Literal(bool can_assign);

  void String(bool can_assign);

  void Variable(bool can_assign);
  void NamedVariable(Token token, bool can_assign);

  void Call(bool can_assign);
  uint8_t ArgumentList();

  void Grouping(bool can_assign);

  void Unary(bool can_assign);

  void Binary(bool can_assign);

  void And(bool can_assign);

  void Or(bool can_assign);

  void Ternary(bool can_assign);

  void Expression();

  void PrintStatement();

  void BlockStatement();

  void IfStatement();

  void WhileStatement();

  void ReturnStatement();

  void ContinueStatement();

  void BreakStatement();

  void ForStatement();

  void SwitchStatement();

  void FunctionStatement(FunctionType type);

  void ExpressionStatement();

  void Statement();

  void VarDeclaration();

  void FunDeclaration();

  void Declaration();
};
