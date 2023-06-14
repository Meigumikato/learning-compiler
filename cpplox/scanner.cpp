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
  while (Peek() != '"' && !IsAtEnd()) {
    if (Peek() == '\n') line++;
    Advance();
  }

  if (Peek() != '"') return ErrorToken("unterminate string.");

  // The closing quote;
  Advance();

  return MakeToken(TokenType::String);
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

  return MakeToken(TokenType::Number);
}
TokenType Scanner::CheckKeyword(int begin, int length, const char* rest, TokenType type) {
  if (begin + length == (int)(current - start) && memcmp(rest, start + begin, length) == 0) {
    return type;
  }

  return TokenType::Identifier;
}

TokenType Scanner::IdentifierType() {
  switch (*start) {
    case 'a':
      return CheckKeyword(1, 2, "nd", TokenType::And);

    case 'b':
      return CheckKeyword(1, 4, "reak", TokenType::Break);

    case 'c': {
      if (current - start > 1) {
        switch (start[1]) {
          case 'l':
            return CheckKeyword(2, 3, "ass", TokenType::Switch);
          case 'a':
            return CheckKeyword(2, 2, "se", TokenType::Case);
          case 'o':
            return CheckKeyword(2, 6, "ntinue", TokenType::Continue);
        }
      }
    }

    case 'e':
      return CheckKeyword(1, 3, "lse", TokenType::Else);
    case 'i':
      return CheckKeyword(1, 1, "f", TokenType::If);
    case 'n':
      return CheckKeyword(1, 2, "il", TokenType::Nil);
    case 'o':
      return CheckKeyword(1, 1, "r", TokenType::Or);
    case 'p':
      return CheckKeyword(1, 4, "rint", TokenType::Print);
    case 'r':
      return CheckKeyword(1, 5, "eturn", TokenType::Return);

    case 's': {
      if (current - start > 1) {
        switch (start[1]) {
          case 'w':
            return CheckKeyword(2, 4, "itch", TokenType::Switch);
          case 'u':
            return CheckKeyword(2, 3, "per", TokenType::Super);
        }
      }
      break;
    }

    case 'v':
      return CheckKeyword(1, 2, "ar", TokenType::Var);
    case 'w':
      return CheckKeyword(1, 4, "hile", TokenType::While);

    case 'f': {
      if (current - start > 1) {
        switch (start[1]) {
          case 'a':
            return CheckKeyword(2, 3, "lse", TokenType::False);
          case 'o':
            return CheckKeyword(2, 1, "r", TokenType::For);
          case 'u':
            return CheckKeyword(2, 1, "n", TokenType::Fun);
        }
      }
      break;
    }

    case 't': {
      if (current - start > 1) {
        switch (start[1]) {
          case 'h':
            return CheckKeyword(2, 2, "is", TokenType::This);
          case 'r':
            return CheckKeyword(2, 2, "ue", TokenType::True);
        }
      }
      break;
    }
  }

  return TokenType::Identifier;
}

Token Scanner::Identifier() {
  while (IsAlpha(Peek()) || IsDigit(Peek())) {
    Advance();
  }

  return MakeToken(IdentifierType());
}

Token Scanner::ScanToken() {
  SkipWhiteSpace();

  start = current;

  if (IsAtEnd()) return MakeToken(TokenType::Eof);

  char c = Advance();

  switch (c) {
    case '(':
      return MakeToken(TokenType::LeftParen);
    case ')':
      return MakeToken(TokenType::RightParen);
    case '}':
      return MakeToken(TokenType::RightBrace);
    case '{':
      return MakeToken(TokenType::LeftBrace);
    case ';':
      return MakeToken(TokenType::Semicolon);
    case ':':
      return MakeToken(TokenType::Colon);
    case ',':
      return MakeToken(TokenType::Comma);
    case '.':
      return MakeToken(TokenType::Dot);
    case '-':
      return MakeToken(TokenType::Minus);
    case '+':
      return MakeToken(TokenType::Plus);
    case '/':
      return MakeToken(TokenType::Slash);
    case '*':
      return MakeToken(TokenType::Star);
    case '?':
      return MakeToken(TokenType::QuestionMark);

    case '!':
      return match('=') ? MakeToken(TokenType::Bang) : MakeToken(TokenType::BangEqual);

    case '=':
      return match('=') ? MakeToken(TokenType::EqualEqual) : MakeToken(TokenType::Equal);
    case '<':
      return match('=') ? MakeToken(TokenType::LessEqual) : MakeToken(TokenType::Less);
    case '>':
      return match('=') ? MakeToken(TokenType::GreaterEqual) : MakeToken(TokenType::Greater);

    case '"':
      return String();

    default:
      if (IsDigit(c)) {
        return Number();
      } else if (IsAlpha(c)) {
        return Identifier();
      }
  }

  return ErrorToken("Unexpected character.");
}
