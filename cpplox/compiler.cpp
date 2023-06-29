#include "compiler.h"

#include "chunk.h"
#include "common.h"
#include "opcode.h"
#include "scanner.h"
#include "value.h"

std::unique_ptr<Function> Compiler::Compile() {
  // TODO: ownership
  FuncScope func_scope(FunctionType::SCRIPT);

  func_scope.enclosing = current_;
  current_ = &func_scope;

  Advance();

  while (!Match(TokenType::Eof)) {
    Declaration();
  }
  Consume(TokenType::Eof, "Expect end of expression.");

  auto function = FinishCompile();

  return parser_.had_error ? nullptr : std::move(function);
}

std::unique_ptr<Function> Compiler::FinishCompile() {
  EmitReturn();

#ifdef DEBUG_PRINT_CODE
  printf("\n======================= compile code ===========================\n");
  if (!parser_.had_error) {
    current_->function->chunk->Disassemble(current_->function->name != nullptr ? current_->function->GetName()
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
  Consume(TokenType::Identifier, msg);

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
  Consume(TokenType::RightParen, "Expect ')' after expression.");
}

void Compiler::Unary(bool can_assign) {
  auto op_type = parser_.previous.type;

  // compile the operand
  ParsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (op_type) {
    case TokenType::Minus:
      EmitByte(+OpCode::OP_NEGATE);
      break;
    case TokenType::Bang:
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
    case TokenType::BangEqual:
      EmitBytes(+OpCode::OP_EQUAL, +OpCode::OP_NOT);
      break;
    case TokenType::EqualEqual:
      EmitByte(+OpCode::OP_EQUAL);
      break;
    case TokenType::Greater:
      EmitByte(+OpCode::OP_GREATER);
      break;
    case TokenType::GreaterEqual:
      EmitBytes(+OpCode::OP_LESS, +OpCode::OP_NOT);
      break;
    case TokenType::Less:
      EmitByte(+OpCode::OP_LESS);
      break;
    case TokenType::LessEqual:
      EmitBytes(+OpCode::OP_GREATER, +OpCode::OP_NOT);
      break;
    case TokenType::Plus:
      EmitByte(+OpCode::OP_ADD);
      break;
    case TokenType::Minus:
      EmitByte(+OpCode::OP_SUBTRACT);
      break;
    case TokenType::Star:
      EmitByte(+OpCode::OP_MULTIPLY);
      break;
    case TokenType::Slash:
      EmitByte(+OpCode::OP_DIVIDE);
      break;
    default:
      return;  // unreachable
  }
}

void Compiler::Ternary(bool can_assign) {
  auto begin = EmitJump(+OpCode::OP_JUMP_IF_FALSE);

  EmitByte(+OpCode::OP_POP);
  ParsePrecedence(PREC_TERNARY);
  auto true_branch = EmitJump(+OpCode::OP_JUMP);

  Consume(TokenType::Colon, "Expect ':' in condtion expression");

  PatchJump(begin);
  EmitByte(+OpCode::OP_POP);
  ParsePrecedence(PREC_TERNARY);

  PatchJump(true_branch);
}

void Compiler::Literal(bool can_assign) {
  switch (parser_.previous.type) {
    case TokenType::False:
      EmitByte(+OpCode::OP_FALSE);
      break;
    case TokenType::Nil:
      EmitByte(+OpCode::OP_NIL);
      break;

    case TokenType::True:
      EmitByte(+OpCode::OP_TRUE);
      break;

    default:
      break;
  }
}

void Compiler::String(bool can_assign) {
  EmitConstant(
      vm_->AllocateString(std::string_view(parser_.previous.start + 1, parser_.previous.length - 2)));
}

void Compiler::Variable(bool can_assign) { NamedVariable(parser_.previous, can_assign); }

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

  if (!Check(TokenType::RightParen)) {
    do {
      Expression();
      arg_count++;

      if (arg_count >= 255) {
        Error("Can't have more than 255 arguments.");
      }

    } while (Match(TokenType::Comma));
  }

  Consume(TokenType::RightParen, "Expect ')' after arguments.");

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
    // local
    get_op = OpCode::OP_GET_LOCAL;
    set_op = OpCode::OP_SET_LOCAL;
  } else if ((arg = ResolveUpvalue(&name)) != -1) {
    // upvalue
    get_op = OpCode::OP_GET_UPVALUE;
    set_op = OpCode::OP_SET_UPVALUE;
  } else {
    // global
    arg = IdentifierConstant(&name);
    get_op = OpCode::OP_GET_GLOBAL;
    set_op = OpCode::OP_SET_GLOBAL;
  }

  if (can_assign && Match(TokenType::Equal)) {
    Expression();
    EmitBytes(+set_op, arg);
  } else {
    EmitBytes(+get_op, arg);
  }
}

void Compiler::Expression() { ParsePrecedence(PREC_ASSIGNMENT); }

void Compiler::BlockStatement() {
  while (!Check(TokenType::Eof) && !Check(TokenType::RightBrace)) {
    Declaration();
  }

  Consume(TokenType::RightBrace, "Expect '}' after block.");
}

void Compiler::IfStatement() {
  Consume(TokenType::LeftParen, "Expect '(' after 'if'.");
  Expression();
  Consume(TokenType::RightParen, "Expect ')' after condition.");

  int then_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
  EmitByte(+OpCode::OP_POP);

  // then branch Statement
  Statement();

  int else_jump = EmitJump(+OpCode::OP_JUMP);

  // if false jump point
  PatchJump(then_jump);
  EmitByte(+OpCode::OP_POP);

  // else branch Statement
  if (Match(TokenType::Else)) Statement();

  PatchJump(else_jump);
}

void Compiler::WhileStatement() {
  int loop_start = current_->function->chunk->Count();
  Consume(TokenType::LeftParen, "Expect '(' after 'while'.");
  Expression();
  Consume(TokenType::RightParen, "Expect ')' after condition.");

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

  if (Match(TokenType::Semicolon)) {
    EmitReturn();
  } else {
    Expression();

    Consume(TokenType::Semicolon, "Expect ';' after return value.");
    EmitByte(+OpCode::OP_RETURN);
  }
}

void Compiler::ContinueStatement() {
  Consume(TokenType::Semicolon, "Expect ';' after continue statement");
  if (current_->loop_depth_ > 0) {
    EmitLoop(current_->loops.back().start);
  } else {
    Error("continue can't use in noun loop inside");
  }
}

void Compiler::BreakStatement() {
  Consume(TokenType::Semicolon, "Expect ';' after break statement.");

  if (current_->loop_depth_ > 0) {
    current_->loops.back().breaks.push_back(EmitJump(+OpCode::OP_JUMP));
  } else {
    Error("break can't use in noun loop inside");
  }
}

// case {constant} : {block}
void Compiler::SwitchStatement() {
  Consume(TokenType::LeftParen, "Expect '(' after 'switch'.");
  Expression();
  Consume(TokenType::RightParen, "Expect ')' after 'switch'.");

  Consume(TokenType::LeftBrace, "Expect '{' after 'switch'.");

  std::vector<int> finish;

  while (Match(TokenType::Case)) {
    Expression();
    Consume(TokenType::Colon, "Expect ':' after 'case'.");
    EmitByte(+OpCode::OP_COMPARE);

    auto cond = EmitJump(+OpCode::OP_JUMP_IF_NO_EQUAL);
    EmitByte(+OpCode::OP_POP);

    if (Check(TokenType::LeftBrace)) {
      BlockStatement();
    } else {
      Statement();
    }

    finish.push_back(EmitJump(+OpCode::OP_JUMP));

    PatchJump(cond);
    EmitByte(+OpCode::OP_POP);
  }

  for (auto f : finish) {
    PatchJump(f);
  }

  // EmitByte(+OpCode::OP_POP);
  Consume(TokenType::RightBrace, "Expect '{' after 'switch'.");

  // pop switch expression
  EmitByte(+OpCode::OP_POP);
}

void Compiler::ForStatement() {
  BeginScope();

  Consume(TokenType::LeftParen, "Expect '(' after 'for'.");
  if (Match(TokenType::Var)) {
    VarDeclaration();
  } else if (Match(TokenType::Semicolon)) {
    // nothing
  } else {
    ExpressionStatement();
  }

  BeginLoop();

  // loop condition
  int loop_start = current_->function->chunk->Count();
  int exit_jump = -1;
  if (!Match(TokenType::Semicolon)) {
    Expression();
    Consume(TokenType::Semicolon, "Expect ';' after loop condtion.");

    // jump out of the loop if the condition is false.
    exit_jump = EmitJump(+OpCode::OP_JUMP_IF_FALSE);
    EmitByte(+OpCode::OP_POP);
  }

  // loop increment
  if (!Match(TokenType::RightParen)) {
    int body_jump = EmitJump(+OpCode::OP_JUMP);
    int increment_start = current_->function->chunk->Count();

    // continue jump point
    current_->loops.back().start = increment_start;

    Expression();
    EmitByte(+OpCode::OP_POP);
    Consume(TokenType::RightParen, "Expect ')' after 'for' clause.");

    EmitLoop(loop_start);
    loop_start = increment_start;
    PatchJump(body_jump);
  }

  // loop body
  Statement();

  current_->loops.back().start = loop_start;
  EmitLoop(loop_start);

  if (exit_jump != -1) {
    PatchJump(exit_jump);
    EmitByte(+OpCode::OP_POP);
  }

  EndLoop();
  EndScope();
}

void Compiler::FunctionStatement(FunctionType type) {
  FuncScope new_func_scope(type);

  if (type == FunctionType::FUNCTION) {
    new_func_scope.function->name =
        AsString(vm_->AllocateString(std::string_view(parser_.previous.start, parser_.previous.length)));
  }

  new_func_scope.enclosing = current_;
  current_ = &new_func_scope;

  BeginScope();

  Consume(TokenType::LeftParen, "");

  if (!Check(TokenType::RightParen)) {
    do {
      current_->function->arity++;
      if (current_->function->arity > 255) {
        Error("Can't have more than 255 parameters.");
      }

      auto constant = ParseVariable("Expect parameter name.");
      DefineVariable(constant);

    } while (Match(TokenType::Comma));
  }

  Consume(TokenType::RightParen, "");
  Consume(TokenType::LeftBrace, "");
  BlockStatement();

  auto function = FinishCompile();

  // // current function upvalue count
  // current_->function->upvalue_count = current_->upvalues.size();

  // function defination instruction (closure)
  EmitBytes(+OpCode::OP_CLOSURE, MakeConstant(function.release()));

  for (auto& upvalue : new_func_scope.upvalues) {
    EmitByte(upvalue.is_local ? 1 : 0);
    EmitByte(upvalue.index);
  }
}

void Compiler::PrintStatement() {
  Expression();
  Consume(TokenType::Semicolon, "Expect ';' after value.");
  EmitByte(+OpCode::OP_PRINT);
}

void Compiler::ExpressionStatement() {
  Expression();
  Consume(TokenType::Semicolon, "Expect ';' after expression.");
  EmitByte(+OpCode::OP_POP);
}

void Compiler::Statement() {
  if (Match(TokenType::Print)) {
    PrintStatement();
  } else if (Match(TokenType::LeftBrace)) {
    BeginScope();
    BlockStatement();
    EndScope();
  } else if (Match(TokenType::If)) {
    IfStatement();
  } else if (Match(TokenType::While)) {
    BeginLoop();
    WhileStatement();
    EndLoop();
  } else if (Match(TokenType::For)) {
    // BeginLoop();
    ForStatement();
    // EndLoop();
  } else if (Match(TokenType::Return)) {
    ReturnStatement();
  } else if (Match(TokenType::Switch)) {
    SwitchStatement();
  } else if (Match(TokenType::Continue)) {
    ContinueStatement();
  } else if (Match(TokenType::Break)) {
    BreakStatement();
  } else {
    ExpressionStatement();
  }
}

void Compiler::VarDeclaration() {
  auto global = ParseVariable("Expect variable name.");

  if (Match(TokenType::Equal)) {
    Expression();
  } else {
    EmitByte(+OpCode::OP_NIL);
  }

  Consume(TokenType::Semicolon, "Expect ';' after variable declaration");

  DefineVariable(global);
}

void Compiler::FunDeclaration() {
  uint8_t global = ParseVariable("Expect function name.");
  MarkInitialized();
  FunctionStatement(FunctionType::FUNCTION);
  DefineVariable(global);
}

void Compiler::Declaration() {
  if (Match(TokenType::Var)) {
    VarDeclaration();
  } else if (Match(TokenType::Fun)) {
    FunDeclaration();
  } else {
    Statement();
  }

  if (parser_.panic_mode) Synchronize();
}
