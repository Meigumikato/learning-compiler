#include "compiler.h"

#include <cstdio>
#include <cstdlib>

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

const Compiler::ParseRule Compiler::rules[(int)TokenType::SENTINAL] = {
    // [(int)TokenType::LEFT_PAREN] =
    {.prefix = &Compiler::Grouping,
     .infix = &Compiler::Call,
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

void Compiler::Advance() {
  parser_.previous = parser_.current;

  for (;;) {
    parser_.current = scanner_.ScanToken();

    if (parser_.current.type != TokenType::ERROR) break;
    ErrorAtCurrent(parser_.current.start);
  }
}

bool Compiler::Match(TokenType type) {
  if (!Check(type)) return false;
  Advance();
  return true;
}

bool Compiler::Check(TokenType type) { return parser_.current.type == type; }

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

ObjFunction* Compiler::FinishCompile() {
  EmitReturn();

#ifdef DEBUG_PRINT_CODE
  if (!parser_.had_error) {
    current_->function->chunk.Disassemble(current_->function->name != nullptr
                                              ? current_->function->name->chars
                                              : "<script>");
  }
#endif

  auto function = current_->function;
  current_ = current_->enclosing;

  return function;
}

ObjFunction* Compiler::Compile() {
  FuncScope func_scope{.function = NewFunction(),
                       .func_type = FunctionType::SCRIPT};

  func_scope.enclosing = current_;
  current_ = &func_scope;

  Advance();

  while (!Match(TokenType::TEOF)) {
    Declaration();
  }
  Consume(TokenType::TEOF, "Expect end of expression.");

  auto function = FinishCompile();

  return parser_.had_error ? nullptr : function;
}

void Compiler::EmitByte(uint8_t byte) {
  current_->function->chunk.Write(byte, parser_.previous.line);
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
  EmitByte(byte1);
  EmitByte(byte2);
}

void Compiler::EmitReturn() {
  EmitByte(OP_NIL);
  EmitByte(OP_RETURN);
}

void Compiler::EmitConstant(Value value) {
  EmitBytes(OP_CONSTANT, MakeConstant(value));
}

uint8_t Compiler::MakeConstant(Value value) {
  int constant = current_->function->chunk.AddConstant(value);
  if (constant > UINT8_MAX) {
    // TODO: OP_CONSTANT_LONG
    Error("Too may constants in one chunk.");
    abort();
  }

  return (uint8_t)constant;
}

uint16_t Compiler::EmitJump(uint8_t instruction) {
  EmitByte(instruction);
  // jump offset 16byte
  EmitBytes(0xff, 0xff);

  return current_->function->chunk.count - 2;
}

void Compiler::PatchJump(int offset) {
  int jump = current_->function->chunk.count - offset - 2;
  if (jump > UINT16_MAX) {
    Error("Too much code to jump over.");
  }

  current_->function->chunk.code[offset] = (jump >> 8) & 0xff;
  current_->function->chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::EmitLoop(int loop_start) {
  EmitByte(OP_LOOP);

  int offset = current_->function->chunk.count - loop_start + 2;
  if (offset > UINT16_MAX) Error("Loop body too large.");

  EmitByte((offset >> 8) & 0xff);
  EmitByte(offset & 0xff);
}

void Compiler::BeginScope() { scope_depth_++; }

void Compiler::EndScope() {
  scope_depth_--;

  while (current_->locals.size() > 0 &&
         current_->locals.back().depth > scope_depth_) {
    EmitByte(OP_POP);
    current_->locals.pop_back();
  }
}

void Compiler::AddLocal(Token name) {
  if (current_->locals.size() == current_->locals.max_size()) {
    Error("Too many local variables in function.");
    return;
  }

  current_->locals.emplace_back(name, -1);
}

uint8_t Compiler::IdentifierConstant(Token* name) {
  return MakeConstant(OBJ_VAL(CopyString(name->start, name->length)));
}

bool Compiler::IdentifierEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

void Compiler::ParsePrecedence(Precedence precedence) {
  Advance();
  auto prefix_rule = GetRule(parser_.previous.type)->prefix;
  if (prefix_rule == nullptr) {
    Error("Expect Expression.");
    return;
  }

  bool can_assign = precedence <= PREC_ASSIGNMENT;
  (this->*prefix_rule)(can_assign);

  while (precedence <= GetRule(parser_.current.type)->precedence) {
    Advance();
    auto infix_rule = GetRule(parser_.previous.type)->infix;
    (this->*infix_rule)(can_assign);
  }
}

void Compiler::DeclareVariable() {
  if (scope_depth_ == 0) return;

  Token* name = &parser_.previous;

  for (int i = current_->locals.size() - 1; i >= 0; --i) {
    auto local = &current_->locals[i];
    if (local->depth != -1 && local->depth < scope_depth_) {
      break;
    }

    if (IdentifierEqual(name, &local->name)) {
      Error("Already a variable with this name in this scope.");
    }
  }
  AddLocal(*name);
}

uint8_t Compiler::ParseVariable(const char* msg) {
  Consume(TokenType::IDENTIFIER, msg);

  DeclareVariable();
  if (scope_depth_ > 0) return 0;

  return IdentifierConstant(&parser_.previous);
}

void Compiler::MarkInitialized() {
  if (scope_depth_ == 0) return;
  current_->locals.back().depth = scope_depth_;
}

void Compiler::DefineVariable(uint8_t global) {
  if (scope_depth_ > 0) {
    MarkInitialized();
    return;
  }
  EmitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::Synchronize() {
  parser_.panic_mode = false;
  while (parser_.current.type != TokenType::TEOF) {
    if (parser_.previous.type == TokenType::SEMICOLON) return;

    switch (parser_.current.type) {
      case TokenType::CLASS:
        [[fallthrough]];
      case TokenType::FUN:
        [[fallthrough]];
      case TokenType::VAR:
        [[fallthrough]];
      case TokenType::FOR:
        [[fallthrough]];
      case TokenType::IF:
        [[fallthrough]];
      case TokenType::WHILE:
        [[fallthrough]];
      case TokenType::PRINT:
        [[fallthrough]];
      case TokenType::RETURN:
        return;

      default:
        break;
    }
    Advance();
  }
}

void Compiler::Number(bool can_assign) {
  double value = strtod(parser_.previous.start, nullptr);
  EmitConstant(NUMBER_VAL(value));
}

void Compiler::Grouping(bool can_assign) {
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::Unary(bool can_assign) {
  auto op_type = parser_.previous.type;

  // compile the operand
  ParsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (op_type) {
    case TokenType::MINUS:
      EmitByte(OP_NEGATE);
      break;
    case TokenType::BANG:
      EmitByte(OP_NOT);
      break;

    default:
      return;
  }
}

void Compiler::Binary(bool can_assign) {
  auto operator_type = parser_.previous.type;
  auto parse_rule = GetRule(operator_type);
  //
  ParsePrecedence((Precedence)((int)(parse_rule->precedence) + 1));
  //
  switch (operator_type) {
    case TokenType::BANG_EQUAL:
      EmitBytes(OP_EQUAL, OP_NOT);
      break;
    case TokenType::EQUAL_EQUAL:
      EmitByte(OP_EQUAL);
      break;
    case TokenType::GREATER:
      EmitByte(OP_GREATER);
      break;
    case TokenType::GREATER_EQUAL:
      EmitBytes(OP_LESS, OP_NOT);
      break;
    case TokenType::LESS:
      EmitByte(OP_LESS);
      break;
    case TokenType::LESS_EQUAL:
      EmitBytes(OP_GREATER, OP_NOT);
      break;
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

void Compiler::Ternary(bool can_assign) {}

void Compiler::Literal(bool can_assign) {
  switch (parser_.previous.type) {
    case TokenType::FALSE:
      EmitByte(OP_FALSE);
      break;
    case TokenType::NIL:
      EmitByte(OP_NIL);
      break;

    case TokenType::TRUE:
      EmitByte(OP_TRUE);
      break;

    default:
      break;
  }
}

void Compiler::String(bool can_assign) {
  EmitConstant(OBJ_VAL(
      CopyString(parser_.previous.start + 1, parser_.previous.length - 2)));
}

void Compiler::Variable(bool can_assign) {
  NamedVariable(parser_.previous, can_assign);
}

int Compiler::ResolveLocal(Token* name) {
  for (int i = current_->locals.size() - 1; i >= 0; i--) {
    auto local = &current_->locals[i];
    if (IdentifierEqual(name, &local->name)) {
      if (local->depth == -1) {
        Error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

void Compiler::And(bool can_assign) {
  int end_jump = EmitJump(OP_JUMP_IF_FALSE);
  EmitByte(OP_POP);
  ParsePrecedence(PREC_AND);

  PatchJump(end_jump);
}

void Compiler::Or(bool can_assign) {
  int else_jump = EmitJump(OP_JUMP_IF_FALSE);
  int end_jump = EmitJump(OP_JUMP);

  PatchJump(else_jump);
  EmitByte(OP_POP);

  ParsePrecedence(PREC_OR);
  PatchJump(end_jump);
}

uint8_t Compiler::ArgumentList() {
  uint8_t arg_count = 0;

  if (!Check(TokenType::RIGHT_PAREN)) {
    do {
      Expression();
      arg_count++;

      if (arg_count >= 255) {
        Error("Can't have more than 255 arguments.");
      }

    } while (Match(TokenType::COMMA));
  }

  Consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

  return arg_count;
}

void Compiler::Call(bool can_assign) {
  auto arg_count = ArgumentList();
  EmitBytes(OP_CALL, arg_count);
}

void Compiler::NamedVariable(Token name, bool can_assign) {
  uint8_t get_op, set_op;

  int arg = ResolveLocal(&name);
  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  } else {
    arg = IdentifierConstant(&name);
    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && Match(TokenType::EQUAL)) {
    Expression();
    EmitBytes(set_op, arg);
  } else {
    EmitBytes(get_op, arg);
  }
}

void Compiler::Expression() { ParsePrecedence(PREC_ASSIGNMENT); }

void Compiler::BlockStatement() {
  while (!Check(TokenType::TEOF) && !Check(TokenType::RIGHT_BRACE)) {
    Declaration();
  }

  Consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::IfStatement() {
  Consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  int then_jump = EmitJump(OP_JUMP_IF_FALSE);
  EmitByte(OP_POP);

  // then branch Statement
  Statement();

  int else_jump = EmitJump(OP_JUMP);

  // if false jump point
  PatchJump(then_jump);
  EmitByte(OP_POP);

  // else branch Statement
  if (Match(TokenType::ELSE)) Statement();

  PatchJump(else_jump);
}

void Compiler::WhileStatement() {
  int loop_start = current_->function->chunk.count;
  Consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  int exit_jump = EmitJump(OP_JUMP_IF_FALSE);
  EmitByte(OP_POP);
  Statement();
  EmitLoop(loop_start);

  PatchJump(exit_jump);
  EmitByte(OP_POP);
}

void Compiler::ReturnStatement() {
  if (current_->func_type == FunctionType::SCRIPT) {
    Error("Can't return from top-level code.");
  }

  if (Match(TokenType::SEMICOLON)) {
    EmitReturn();
  } else {
    Expression();

    Consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    EmitByte(OP_RETURN);
  }
}

void Compiler::ForStatement() {
  BeginScope();

  Consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  if (Match(TokenType::VAR)) {
    VarDeclaration();
  } else if (Match(TokenType::SEMICOLON)) {
    // nothing
  } else {
    ExpressionStatement();
  }

  // loop condition
  int loop_start = current_->function->chunk.count;
  int exit_jump = -1;
  if (!Match(TokenType::SEMICOLON)) {
    Expression();
    Consume(TokenType::SEMICOLON, "Expect ';' after loop condtion.");

    // jump out of the loop if the condition is false.
    exit_jump = EmitJump(OP_JUMP_IF_FALSE);
    EmitByte(OP_POP);
  }

  // loop increment
  if (!Match(TokenType::RIGHT_PAREN)) {
    int body_jump = EmitJump(OP_JUMP);
    int increment_start = current_->function->chunk.count;
    Expression();
    EmitByte(OP_POP);
    Consume(TokenType::RIGHT_PAREN, "Expect ')' after 'for' clause.");

    EmitLoop(loop_start);
    loop_start = increment_start;
    PatchJump(body_jump);
  }

  // loop body
  Statement();

  EmitLoop(loop_start);

  if (exit_jump != -1) {
    PatchJump(exit_jump);
    EmitByte(OP_POP);
  }

  EndScope();
}

void Compiler::Function(FunctionType type) {
  FuncScope new_func_scope{.function = NewFunction(), .func_type = type};

  if (new_func_scope.func_type == FunctionType::FUNCTION) {
    new_func_scope.function->name =
        CopyString(parser_.previous.start, parser_.previous.length);
  }

  new_func_scope.enclosing = current_;
  current_ = &new_func_scope;

  BeginScope();

  Consume(TokenType::LEFT_PAREN, "");

  if (!Check(TokenType::RIGHT_PAREN)) {
    do {
      current_->function->arity++;
      if (current_->function->arity > 255) {
        Error("Can't have more than 255 parameters.");
      }

      auto constant = ParseVariable("Expect parameter name.");
      DefineVariable(constant);

    } while (Match(TokenType::COMMA));
  }

  Consume(TokenType::RIGHT_PAREN, "");
  Consume(TokenType::LEFT_BRACE, "");
  BlockStatement();

  ObjFunction* function = FinishCompile();
  EmitBytes(OP_CONSTANT, MakeConstant(OBJ_VAL(function)));
}

void Compiler::PrintStatement() {
  Expression();
  Consume(TokenType::SEMICOLON, "Expect ';' after value.");
  EmitByte(OP_PRINT);
}

void Compiler::ExpressionStatement() {
  Expression();
  Consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  EmitByte(OP_POP);
}

void Compiler::Statement() {
  if (Match(TokenType::PRINT)) {
    PrintStatement();
  } else if (Match(TokenType::LEFT_BRACE)) {
    BeginScope();
    BlockStatement();
    EndScope();
  } else if (Match(TokenType::IF)) {
    IfStatement();
  } else if (Match(TokenType::WHILE)) {
    WhileStatement();
  } else if (Match(TokenType::RETURN)) {
    ReturnStatement();
  } else {
    ExpressionStatement();
  }
}

void Compiler::VarDeclaration() {
  auto global = ParseVariable("Expect variable name.");

  if (Match(TokenType::EQUAL)) {
    Expression();
  } else {
    EmitByte(OP_NIL);
  }

  Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");

  DefineVariable(global);
}

void Compiler::FunDeclaration() {
  uint8_t global = ParseVariable("Expect function name.");
  MarkInitialized();
  Function(FunctionType::FUNCTION);
  DefineVariable(global);
}

void Compiler::Declaration() {
  if (Match(TokenType::VAR)) {
    VarDeclaration();
  } else if (Match(TokenType::FUN)) {
    FunDeclaration();
  } else {
    Statement();
  }

  if (parser_.panic_mode) Synchronize();
}
