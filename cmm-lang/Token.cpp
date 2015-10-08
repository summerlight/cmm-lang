#include "StdAfx.h"
#include "Token.h"

namespace cmm
{

const std::wstring Token::TokenNameTable_[] = {
	// Tokens for arithmetic operator
	L"ARITH_ADD",      // +
	L"ARITH_SUB",      // -
	L"ARITH_MUL",      // *
	L"ARITH_DIV",      // /
	L"ARITH_MOD",      // %
	L"ARITH_INC",      // ++
	L"ARITH_DEC",      // --

	// Tokens for bitwise operator
	L"BIT_AND",        // &
	L"BIT_OR",         // |
	L"BIT_NOT",        // ~
	L"BIT_XOR",        // ^
	L"BIT_SL",         // <<
	L"BIT_SR",         // >>

	// Tokens for assignment operator
	L"ASSIGN",         // =
	L"ASSIGN_ADD",     // +=
	L"ASSIGN_SUB",   // -=
	L"ASSIGN_MUL",     // *=
	L"ASSIGN_DIV",     // /=
	L"ASSIGN_MOD",     // %=
	L"ASSIGN_SL",      // <<=
	L"ASSIGN_SR",      // >>=
	L"ASSIGN_AND",     // &=
	L"ASSIGN_OR",      // !=
	L"ASSIGN_XOR",     // ^=
	
	// Tokens for logical operator
	L"LOGIC_OR",       // ||
	L"LOGIC_AND",      // &&
	L"LOGIC_NOT",      // !
	L"LOGIC_EQ",       // ==
	L"LOGIC_NOTEQ",    // !=
	L"LOGIC_GREATER",  // >
	L"LOGIC_GE",       // >=
	L"LOGIC_LESS",     // <
	L"LOGIC_LE",       // <=
	L"LOGIC_IF",       // ?

	// Token for puntuator
	L"LEFTBRACE",      // {
    L"RIGHTBRACE",     // }
    L"LEFTBRACKET",    // [
    L"RIGHTBRACKET",   // ]
    L"LEFTPAREN",      // (
    L"RIGHTPAREN",     // )
    L"COMMA",          // ,
	L"DOT",            // ,
    L"SEMICOLON",      // ;
	L"COLON",          // :

	// Token for numeric and string literal representation
	L"LITERAL_INT",    // ex. 4125, -125 ...
	L"LITERAL_HEX",    // ex. 0x4125, -0x125 ...
    L"LITERAL_FLOAT",  // ex. 0.15, 12f ... 
	L"LITERAL_STRING", // ex. "string"

	// Token for identifier
	L"IDENTIFIER",

	// Token for keywords
	L"KEYWORD_NULL",
	L"KEYWORD_ARRAY",
	L"KEYWORD_TABLE",
	L"KEYWORD_FUNCTION",
	L"KEYWORD_TRUE",
	L"KEYWORD_FALSE",
	L"KEYWORD_BREAK",
	L"KEYWORD_CONTINUE",
	L"KEYWORD_IF",
	L"KEYWORD_ELSE",
	L"KEYWORD_DO",
	L"KEYWORD_WHILE",
	L"KEYWORD_FOR",
	L"KEYWORD_FOREACH",
	L"KEYWORD_RETURN",
	L"KEYWORD_YIELD",
	L"KEYWORD_LOCAL",

	// Special token
	L"ERROR",
	L"END OF CODE"
};


const std::wstring Token::LexemeTable_[] = {
	// Tokens for arithmetic operator
	L"+",
	L"-",
	L"*",
	L"/",
	L"%",
	L"++",
	L"--",

	// Tokens for bitwise operator
	L"&",
	L"|",
	L"~",
	L"^",
	L"<<",
	L">>",

	// Tokens for assignment operator
	L"+",
	L"+=",
	L"-=",
	L"*=",
	L"/=",
	L"%=",
	L"<<=",
	L">>=",
	L"&=",
	L"!=",
	L"^=",
	
	// Tokens for logical operator
	L"||",
	L"&&",
	L"!",
	L"==",
	L"!=",
	L">",
	L">=",
	L"<",
	L"<=",
	L"?",

	// Token for puntuator
	L"{",
	L"}",
	L"{",
	L"}",
    L"(",
    L")",
    L",",
	L".",
    L";",
	L":",

	// Numeric, string literals and identifiers don't have fixed lexeme
	L"<LITERAL_INT>",
	L"<LITERAL_HEX>",
	L"<LITERAL_FLOAT>",
	L"<LITERAL_STRING>",
	L"<IDENTIFIER>",

	// Token for keywords
	L"null",
	L"array",
	L"table",
	L"function",
	L"true",
	L"false",
	L"break",
	L"continue",
	L"if",
	L"else",
	L"do",
	L"while",
	L"for",
	L"foreach",
	L"return",
	L"yield",
	L"local",

	// Special token
	L"<ERROR>",
	L"<EOF>"
};

} // The end of namespace "cmm"