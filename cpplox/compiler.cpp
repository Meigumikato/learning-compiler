#include "compiler.h"

#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "opcode.h"
#include "scanner.h"
#include "value.h"

std::unique_ptr<Function> Compiler::Compile() {
  // TODO: ownership
  FuncScope func_scope(FunctionType::SCRIPT);

  func_scope.enclosing = current_;
  current_ = &func_scope;

  Advance();

  while (!Match(TokenType::TEOF)) {
    Declaration();
  }
  Consume(TokenType::TEOF, "Expect end of expression.");

  auto function = FinishCompile();

  return parser_.had_error ? nullptr : std::move(function);
}

std::unique_ptr<Function> Compiler::FinishCompile() {
  EmitReturn();

#ifdef DEBUG_PRINT_CODE
  if (!parser_.had_error) {
    current_->function->chunk.Disassemble(
        current_->function->name != nullptr
            ? current_->function->name->GetCString()
            : "<script>");
  }
#endif

  auto function = std::move(current_->function);
  current_ = current_->enclosing;

  return function;
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
  if (current_->scope_depth_ == 0) return;

  Token* name = &parser_.previous;

  for (int i = current_->locals.size() - 1; i >= 0; --i) {
    auto local = &current_->locals[i];
    if (local->depth != -1 && local->depth < current_->scope_depth_) {
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

  if (current_->scope_depth_ > 0) return 0;

  return IdentifierConstant(&parser_.previous);
}

void Compiler::DefineVariable(uint8_t global) {
  if (current_->scope_depth_ > 0) {
    MarkInitialized();
    return;
  }
  EmitBytes(+OpCode::OP_DEFINE_GLOBAL, global);
}

void Compiler::Number(bool can_assign) {
  double value = strtod(parser_.previous.start, nullptr);
  EmitConstant(value);
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
      EmitByte(+OpCode::OP_NEGATE);
      break;
    case TokenType::BANG:
      EmitByte(+OpCode::OP_NOT);
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
      EmitBytes(+OpCode::OP_EQUAL, +OpCode::OP_NOT);
      break;
    case TokenType::EQUAL_EQUAL:
      EmitByte(+OpCode::OP_EQUAL);
      break;
    case TokenType::GREATER:
      EmitByte(+OpCode::OP_GREATER);
      break;
    case TokenType::GREATER_EQUAL:
      EmitBytes(+OpCode::OP_LESS, +OpCode::OP_NOT);
      break;
    case TokenType::LESS:
      EmitByte(+OpCode::OP_LESS);
      break;
    case TokenType::LESS_EQUAL:
      EmitBytes(+OpCode::OP_GREATER, +OpCode::OP_NOT);
      break;
    case TokenType::PLUS:
      EmitByte(+OpCode::OP_ADD);
      break;
    case TokenType::MINUS:
      EmitByte(+OpCode::OP_SUBTRACT);
      break;
    case TokenType::STAR:
      EmitByte(+OpCode::OP_MULTIPLY);
      break;
    case TokenType::SLASH:
      EmitByte(+OpCode::OP_DIVIDE);
      break;
    default:
      return;  // unreachable
  }
}

void Compiler::Ternary(bool can_assign) {}

void Compiler::Literal(bool can_assign) {
  switch (parser_.previous.type) {
    case TokenType::FALSE:
      EmitByte(+OpCode::OP_FALSE);
      break;
    case TokenType::NIL:
      EmitByte(+OpCode::OP_NIL);
      break;

    case TokenType::TRUE:
      EmitByte(+OpCode::OP_TRUE);
      break;

    default:
      break;
  }
}

void Compiler::String(bool can_assign) {
  EmitConstant(vm_->AllocateString(std::string_view(
      parser_.previous.start + 1, parser_.previous.length - 2)));
}

void Compiler::Variable(bool can_assign) {
  NamedVariable(parser_.previous, can_assign);
}

void Compiler::And(bool can_assign) {
  int end_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
  EmitByte(+OpCode::OP_POP);
  ParsePrecedence(PREC_AND);

  PatchJump(end_jump);
}

void Compiler::Or(bool can_assign) {
  int else_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
  int end_jump = EmitJump(+OpCode::OP_JUMP);

  PatchJump(else_jump);
  EmitByte(+OpCode::OP_POP);

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
  EmitBytes(+OpCode::OP_CALL, arg_count);
}

void Compiler::NamedVariable(Token name, bool can_assign) {
  OpCode get_op, set_op;

  int arg = ResolveLocal(&name);
  if (arg != -1) {
    get_op = OpCode::OP_GET_LOCAL;
    set_op = OpCode::OP_SET_LOCAL;
  } else {
    arg = IdentifierConstant(&name);
    get_op = OpCode::OP_GET_GLOBAL;
    set_op = OpCode::OP_SET_GLOBAL;
  }

  if (can_assign && Match(TokenType::EQUAL)) {
    Expression();
    EmitBytes(+set_op, arg);
  } else {
    EmitBytes(+get_op, arg);
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

  int then_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
  EmitByte(+OpCode::OP_POP);

  // then branch Statement
  Statement();

  int else_jump = EmitJump(+OpCode::OP_JUMP);

  // if false jump point
  PatchJump(then_jump);
  EmitByte(+OpCode::OP_POP);

  // else branch Statement
  if (Match(TokenType::ELSE)) Statement();

  PatchJump(else_jump);
}

void Compiler::WhileStatement() {
  int loop_start = current_->function->chunk.Count();
  Consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  Expression();
  Consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  int exit_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
  EmitByte(+OpCode::OP_POP);
  Statement();
  EmitLoop(loop_start);

  PatchJump(exit_jump);
  EmitByte(+OpCode::OP_POP);
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
    EmitByte(+OpCode::OP_RETURN);
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
  int loop_start = current_->function->chunk.Count();
  int exit_jump = -1;
  if (!Match(TokenType::SEMICOLON)) {
    Expression();
    Consume(TokenType::SEMICOLON, "Expect ';' after loop condtion.");

    // jump out of the loop if the condition is false.
    exit_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
    EmitByte(+OpCode::OP_POP);
  }

  // loop increment
  if (!Match(TokenType::RIGHT_PAREN)) {
    int body_jump = EmitJump(+OpCode::OP_JUMP);
    int increment_start = current_->function->chunk.Count();
    Expression();
    EmitByte(+OpCode::OP_POP);
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
    EmitByte(+OpCode::OP_POP);
  }

  EndScope();
}

void Compiler::FunctionStatement(FunctionType type) {
  FuncScope new_func_scope(type);

  if (type == FunctionType::FUNCTION) {
    new_func_scope.function->name = AsString(vm_->AllocateString(
        std::string_view(parser_.previous.start, parser_.previous.length)));
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

  auto function = FinishCompile();
  EmitBytes(+OpCode::OP_CONSTANT, MakeConstant(function.release()));
}

void Compiler::PrintStatement() {
  Expression();
  Consume(TokenType::SEMICOLON, "Expect ';' after value.");
  EmitByte(+OpCode::OP_PRINT);
}

void Compiler::ExpressionStatement() {
  Expression();
  Consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  EmitByte(+OpCode::OP_POP);
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
    EmitByte(+OpCode::OP_NIL);
  }

  Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");

  DefineVariable(global);
}

void Compiler::FunDeclaration() {
  uint8_t global = ParseVariable("Expect function name.");
  MarkInitialized();
  FunctionStatement(FunctionType::FUNCTION);
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
