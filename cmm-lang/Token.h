#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <utility>

#include "Position.h"

using std::wstring;

namespace cmm
{

class Token
{
public:
	enum Type {		
		// Tokens for arithmetic operator
		ARITH_ADD,      // +
		ARITH_SUB,      // -
		ARITH_MUL,      // *
		ARITH_DIV,      // /
		ARITH_MOD,      // %
		ARITH_INC,      // ++
		ARITH_DEC,      // --

		// Tokens for bitwise operator
		BIT_AND,        // &
		BIT_OR,         // |
		BIT_NOT,        // ~
		BIT_XOR,        // ^
		BIT_SL,         // <<
		BIT_SR,         // >>

		// Tokens for assignment operator
		ASSIGN,         // =
		ASSIGN_ADD,     // +=
		ASSIGN_SUB,     // -=
		ASSIGN_MUL,     // *=
		ASSIGN_DIV,     // /=
		ASSIGN_MOD,     // %=
		ASSIGN_SL,      // <<=
		ASSIGN_SR,      // >>=
		ASSIGN_AND,     // &=
		ASSIGN_OR,      // !=
		ASSIGN_XOR,     // ^=

		// Tokens for logical operator
		LOGIC_OR,       // ||
		LOGIC_AND,      // &&
		LOGIC_NOT,      // !
		LOGIC_EQ,       // ==
		LOGIC_NOTEQ,    // !=
		LOGIC_GREATER,  // >
		LOGIC_GE,       // >=
		LOGIC_LESS,     // <
		LOGIC_LE,       // <=
		LOGIC_IF,       // ?

		// Token for puntuator
		LEFTBRACE,      // {
		RIGHTBRACE,     // }
		LEFTBRACKET,    // [
		RIGHTBRACKET,   // ]
		LEFTPAREN,      // (
		RIGHTPAREN,     // )
		COMMA,          // ,
		DOT,            // .
		SEMICOLON,      // ;
		COLON,          // :

		// Token for numeric and string literal representation
		LITERAL_INT,    // ex. 4125, -125 ...
		LITERAL_HEX,    // ex. 0x4125, -0x125 ...
		LITERAL_FLOAT,  // ex. 0.15, 12f ... 
		LITERAL_STRING, // ex. "string"

		// Token for identifier
		IDENTIFIER,

		// Token for keywords
		KEYWORD_NULL,
		KEYWORD_ARRAY,
		KEYWORD_TABLE,
		KEYWORD_FUNCTION,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
		KEYWORD_BREAK,
		KEYWORD_CONTINUE,
		KEYWORD_IF,
		KEYWORD_ELSE,
		KEYWORD_DO,
		KEYWORD_WHILE,
		KEYWORD_FOR,
		KEYWORD_FOREACH,
		KEYWORD_RETURN,
		KEYWORD_YIELD,
		KEYWORD_LOCAL,

		// Special token
		ERR,
		END
	};

	explicit                    Token() { /* Intentionally empty function*/ };
	                            Token(const Type type, const wstring& lex, const Position& pos);

	                            Token(const Token&& token);
	const Token&                operator=(const Token&& token);

	const Position&             pos() const;
	const std::wstring&         lexeme() const;
	const std::wstring&         name() const;
	const Type                  type() const;

	const bool                  isAssignOp() const;
	const bool                  isEqualityOp() const;
	const bool                  isRelationalOp() const;
	const bool                  isShiftOp() const;
	const bool                  isAdditiveOp() const;
	const bool                  isMultiplicativeOp() const;
	const bool                  isUnaryOp() const;
	const bool                  isPostfixOp() const;

	static const std::wstring&  lexeme(Type t);

private:
	                            Token(const Token&);
	const Token&                operator=(const Token&);

	static const std::wstring   TokenNameTable_[];
	static const std::wstring   LexemeTable_[];

	Type                        type_;
	Position                    position_;
	wstring                     lexeme_;	
};

inline Token::Token(const Type type, const wstring& lex, const Position& pos)
	: type_(type), lexeme_(lex), position_(pos)
{
	// Intentionally empty function
}

inline Token::Token(const Token&& token)
: type_(token.type_), position_(token.position_), lexeme_(std::move(token.lexeme_))
{
}

inline const Token& Token::operator=(const Token&& token)
{
	type_ = token.type_;
	position_ = token.position_;
	lexeme_ = std::move(token.lexeme_);

	return *this;
}

inline const Position& Token::pos() const
{
	return position_;
}

inline const std::wstring& Token::lexeme() const
{
	return lexeme_;
}

inline const std::wstring& Token::name() const
{
	return TokenNameTable_[type_];
}

inline const Token::Type Token::type() const
{
	return type_;
}

inline const std::wstring& Token::lexeme(Type type)
{
	return LexemeTable_[type];
}

inline const bool Token::isAssignOp() const
{
	return (type_ >= ASSIGN) && (type_ <= ASSIGN_XOR);
}

inline const bool Token::isEqualityOp() const
{
	return (type_ == LOGIC_EQ) || (type_ == LOGIC_NOTEQ);
}

inline const bool Token::isRelationalOp() const
{
	return (type_ >= LOGIC_GREATER) || (type_ <= LOGIC_LE);
}

inline const bool Token::isShiftOp() const
{
	return (type_ == BIT_SL) || (type_ == BIT_SR);
}

inline const bool Token::isAdditiveOp() const
{
	return (type_ == ARITH_ADD) || (type_ == ARITH_SUB);
}

inline const bool Token::isMultiplicativeOp() const
{
	return (type_ == ARITH_MUL) || (type_ == ARITH_DIV);
}

inline const bool Token::isUnaryOp() const
{
	return (type_ == ARITH_ADD) || (type_ == ARITH_SUB) ||
	       (type_ == ARITH_INC) || (type_ == ARITH_DEC) ||
	       (type_ == BIT_NOT) || (type_ == LOGIC_NOT);
}

inline const bool Token::isPostfixOp() const
{
	return (type_ == LEFTBRACKET) || (type_ == LEFTPAREN) ||
	       (type_ == ARITH_INC) || (type_ == ARITH_DEC);
}

} // The end of namespace "cmm"

#endif