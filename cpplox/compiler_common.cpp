#include "compiler.h"
#include "opcode.h"

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

void Compiler::EmitByte(uint8_t byte) {
  current_->function->chunk.Write(byte, parser_.previous.line);
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
  EmitByte(byte1);
  EmitByte(byte2);
}

void Compiler::EmitReturn() {
  EmitByte(static_cast<uint8_t>(OpCode::OP_NIL));
  EmitByte(static_cast<uint8_t>(OpCode::OP_RETURN));
}

void Compiler::EmitConstant(Value value) {
  EmitBytes(static_cast<uint8_t>(OpCode::OP_CONSTANT), MakeConstant(value));
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

  return current_->function->chunk.Count() - 2;
}

void Compiler::PatchJump(int offset) {
  int jump = current_->function->chunk.Count() - offset - 2;
  if (jump > UINT16_MAX) {
    Error("Too much code to jump over.");
  }

  current_->function->chunk.code[offset] = (jump >> 8) & 0xff;
  current_->function->chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::EmitLoop(int loop_start) {
  EmitByte(static_cast<uint8_t>(OpCode::OP_LOOP));

  int offset = current_->function->chunk.Count() - loop_start + 2;
  if (offset > UINT16_MAX) Error("Loop body too large.");

  EmitByte((offset >> 8) & 0xff);
  EmitByte(offset & 0xff);
}

void Compiler::BeginScope() { current_->scope_depth_++; }

void Compiler::EndScope() {
  current_->scope_depth_--;

  while (current_->locals.size() > 0 &&
         current_->locals.back().depth > current_->scope_depth_) {
    EmitByte(static_cast<uint8_t>(OpCode::OP_POP));
    current_->locals.pop_back();
  }
}

void Compiler::AddLocal(Token name) {
  if (current_->locals.size() == current_->locals.max_size()) {
    Error("Too many local variables in function.");
    return;
  }

  current_->locals.push_back(Local{.name = name, .depth = -1});
}

uint8_t Compiler::IdentifierConstant(Token* name) {
  return MakeConstant(CopyString(name->start, name->length));
}

bool Compiler::IdentifierEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
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

void Compiler::MarkInitialized() {
  if (current_->scope_depth_ == 0) return;
  current_->locals.back().depth = current_->scope_depth_;
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
