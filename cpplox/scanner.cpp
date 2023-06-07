#include "scanner.h"

#include <cstdio>
#include <cstring>

#include "common.h"

void Scanner::SkipWhiteSpace() {
  for (;;) {
    char c = Peek();

    switch (c) {
      case '\n':
        line++;
        [[fallthrough]];
      case ' ':
        [[fallthrough]];
      case '\t':
        [[fallthrough]];
      case '\r':
        Advance();
        break;

      case '/': {
        if (PeekNext() == '/') {
          while (Peek() != '\n' && !IsAtEnd()) Advance();
        } else {
          return;
        }
        break;
      }

      default:
        return;
    }
  }
}

Token Scanner::String() {
  while (Peek() != '"' && IsAtEnd()) {
    if (Peek() == '\n') line++;
    Advance();
  }

  if (Peek() != '"') return ErrorToken("unterminate string.");

  // The closing quote;
  Advance();

  return MakeToken(TokenType::STRING);
}

Token Scanner::Number() {
  while (!IsAtEnd() && IsDigit(Peek())) {
    Advance();
  }

  if (Peek() == '.' && IsDigit(PeekNext())) {
    // consume dot
    Advance();

    while (!IsAtEnd() && IsDigit(Peek())) {
      Advance();
    }
  }

  return MakeToken(TokenType::NUMBER);
}
TokenType Scanner::CheckKeyword(int begain, int length, const char* rest,
                                TokenType type) {
  if (begain + length == (int)(current - start) &&
      memcmp(rest, start + begain, length) == 0) {
    return type;
  }

  return TokenType::IDENTIFIER;
}

TokenType Scanner::IdentifierType() {
  switch (*start) {
    case 'a':
      return CheckKeyword(1, 2, "nd", TokenType::AND);
    case 'c':
      return CheckKeyword(1, 4, "lass", TokenType::CLASS);
    case 'e':
      return CheckKeyword(1, 3, "lse", TokenType::ELSE);
    case 'i':
      return CheckKeyword(1, 1, "f", TokenType::IF);
    case 'n':
      return CheckKeyword(1, 2, "il", TokenType::NIL);
    case 'o':
      return CheckKeyword(1, 1, "r", TokenType::OR);
    case 'p':
      return CheckKeyword(1, 4, "rint", TokenType::PRINT);
    case 'r':
      return CheckKeyword(1, 5, "eturn", TokenType::RETURN);
    case 's':
      return CheckKeyword(1, 4, "uper", TokenType::SUPER);
    case 'v':
      return CheckKeyword(1, 2, "ar", TokenType::VAR);
    case 'w':
      return CheckKeyword(1, 4, "hile", TokenType::WHILE);

    case 'f':
      if (current - start > 1) {
        switch (start[1]) {
          case 'a':
            return CheckKeyword(2, 3, "lse", TokenType::FALSE);
          case 'o':
            return CheckKeyword(2, 1, "r", TokenType::FOR);
          case 'u':
            return CheckKeyword(2, 1, "n", TokenType::FUN);
        }
      }
      break;

    case 't':
      if (current - start > 1) {
        switch (start[1]) {
          case 'h':
            return CheckKeyword(2, 2, "is", TokenType::THIS);
          case 'r':
            return CheckKeyword(2, 2, "ue", TokenType::TRUE);
        }
      }
      break;
  }

  return TokenType::IDENTIFIER;
}

Token Scanner::Identifier() {
  while (IsAlpha(Peek()) || IsDigit(Peek())) {
    Advance();
  }

  return MakeToken(IdentifierType());
}

Token Scanner::ScanToken() {
  start = current;

  if (IsAtEnd()) return MakeToken(TokenType::TEOF);

  char c = Advance();

  switch (c) {
    case '(':
      return MakeToken(TokenType::LEFT_PAREN);
    case ')':
      return MakeToken(TokenType::RIGHT_PAREN);
    case '}':
      return MakeToken(TokenType::LEFT_BRACE);
    case '{':
      return MakeToken(TokenType::RIGHT_BRACE);
    case ';':
      return MakeToken(TokenType::SEMICOLON);
    case ',':
      return MakeToken(TokenType::COMMA);
    case '.':
      return MakeToken(TokenType::DOT);
    case '-':
      return MakeToken(TokenType::MINUS);
    case '+':
      return MakeToken(TokenType::PLUS);
    case '/':
      return MakeToken(TokenType::SLASH);
    case '*':
      return MakeToken(TokenType::STAR);

    case '!':
      return match('=') ? MakeToken(TokenType::BANG)
                        : MakeToken(TokenType::BANG_EQUAL);

    case '=':
      return match('=') ? MakeToken(TokenType::EQUAL_EQUAL)
                        : MakeToken(TokenType::EQUAL);
    case '<':
      return match('=') ? MakeToken(TokenType::LESS_EQUAL)
                        : MakeToken(TokenType::LESS);
    case '>':
      return match('=') ? MakeToken(TokenType::GREATER_EQUAL)
                        : MakeToken(TokenType::GREATER);

    case '"':
      return String();

    default:
      if (IsDigit(c)) {
        return Number();
      } else if (IsAlpha(c)) {
        return Identifier();
      }

      // case '(':
      // case '(':
  }

  return ErrorToken("Unexpected character.");
}
