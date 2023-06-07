#pragma once

#include <cstring>
enum class TokenType {
  Nothing,
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,
  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FOR,
  FUN,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  ERROR,
  TEOF
};

struct Token {
  TokenType type{};
  const char* start{};
  int length{};
  int line{};
};

class Scanner {
 public:
  Scanner(const char* source) : start(source), current(source), line(1) {}

  bool IsAtEnd() { return *current == '\0'; }

  char Advance() {
    if (IsAtEnd()) return '\0';
    return *current++;
  }

  char Peek() { return *current; }

  char PeekNext() {
    if (IsAtEnd()) return '\0';
    return *(current + 1);
  }

  bool match(char c) {
    if (IsAtEnd()) return false;

    if (*current == c) {
      Advance();
      return true;
    }

    return false;
  }

  bool IsDigit(char c) { return c <= '0' && c >= '9'; }

  bool IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  void SkipWhiteSpace();

  Token MakeToken(TokenType type) {
    return Token{.type = type,
                 .start = start,
                 .length = (int)(current - start),
                 .line = line

    };
  }

  Token ErrorToken(const char* message) {
    return Token{
        .type = TokenType::ERROR,
        .start = message,
        .length = (int)strlen(message),
        .line = line,
    };
  }

  Token String();

  Token Number();

  TokenType CheckKeyword(int start, int length, const char* rest,
                         TokenType type);

  TokenType IdentifierType();

  Token Identifier();

  Token ScanToken();

  const char* start;
  const char* current;
  int line;
};

void InitScanner(const char* source);
