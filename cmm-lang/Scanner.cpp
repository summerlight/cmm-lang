#include "StdAfx.h"
#include "Scanner.h"

#include <cassert>
#include <cstdint>
#include <cwchar>
#include <cstdio>

#include "Error.h"


namespace {

inline bool isPunctuator(wchar_t c)
{
	return ((c >= 0x21) && (c <= 0x2f)) || // ! " # $ % & ' ( ) * + , - . /
	       ((c >= 0x3a) && (c <= 0x40)) || // : ; < = > ? @
	       ((c >= 0x5b) && (c <= 0x5e)) || // [ \ ] ^
	       ((c >= 0x7b) && (c <= 0x7e));   // { | } ~
}

inline bool isDigit(wchar_t c)
{
	return ((c >= 0x30) && (c <= 0x39)); // 0 ~ 9
}

inline bool isHexadecimal(wchar_t c)
{
	return ((c >= 0x30) && (c <= 0x39)) || // 0 ~ 9
	       ((c >= 0x41) && (c <= 0x46)) || // A ~ F
	       ((c >= 0x61) && (c <= 0x66));   // a ~ f
}

inline bool isOctal(wchar_t c)
{
	return ((c >= 0x30) && (c <= 0x37)); // 0 ~ 7
}

}


namespace cmm
{

Scanner::Scanner()
: code_(nullptr), codeSize_(0), isScanning_(true)
{
}

Scanner::~Scanner()
{
}

Token Scanner::scan()
{
	if (code_ == nullptr) {
		return Token(Token::ERR, L"", Position(0, 0, 0, 0));
	}

	currentLexeme_.clear();
	skipBlankAndComment_();

	uint32_t startLine = currentLine_;
	uint32_t startCol = currentCol_;

	Token::Type type = scanToken_();
	
	return Token(type, currentLexeme_, Position(startLine, currentLine_, startCol, currentCol_-1));
}


bool Scanner::load(const wchar_t code[])
{
	return load(code, wcslen(code));
}


bool Scanner::load(const wchar_t code[], uint32_t codeSize)
{
	code_ = code;
	codeSize_ = codeSize;
	currentOffset_ = 0;
	currentLine_ = 1;
	currentCol_ = 1;
	
	return true;
}


inline wchar_t Scanner::currentChar_()
{
	return (currentOffset_ < codeSize_) ? code_[currentOffset_] : 0;
}

inline wchar_t Scanner::lookForward_(uint32_t n)
{
	return (currentOffset_ + n < codeSize_) ? code_[currentOffset_ + n] : 0;
}

void Scanner::processCurrentChar_()
{	
	if (isScanning_)
		currentLexeme_.push_back(currentChar_());

	if (currentChar_() == L'\n') {
		currentLine_++;
		currentCol_ = 1;
	} else if (currentChar_() != L'\0') {
		// TODO : should we check current character is full-width or not?
		if (currentChar_() == L'\t') {
			currentCol_ += SCANNER_TAP_SIZE;
		} else {
			currentCol_++;
		}
	}
	
	currentOffset_++;
}

inline void Scanner::skipCurrentChar_()
{
	bool isScanningTemp = isScanning_;
	isScanning_ = false;
	processCurrentChar_();
	isScanning_ = isScanningTemp;
}

inline void Scanner::replaceCurrentChar_(const wchar_t replacement)
{
	skipCurrentChar_();
	currentLexeme_.push_back(replacement);
}

void Scanner::skipBlankAndComment_()
{
	isScanning_ = false;
		
	for(;;) {
		if (iswspace(currentChar_())) {
			processCurrentChar_();
		} else if (currentChar_() == L'/') {
			if(lookForward_(1) == L'*') {
				processCurrentChar_();
				
				for(;;) {
					processCurrentChar_();					
					if (currentChar_() == L'*' && lookForward_(1) == L'/') {
						processCurrentChar_();
						processCurrentChar_();						
						break;
					} else if (currentChar_() == 0) { // EOF
						throw Error(L"%d: unterminated multi-line comment", currentLine_);
						break;
					}
				}
			} else if (lookForward_(1) == L'/') {
				processCurrentChar_();				
				for(;;) {
					processCurrentChar_();				
					if (currentChar_() == L'\n') {
						processCurrentChar_();						
						break;
					}
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}

	isScanning_ = true;
}

Token::Type Scanner::scanToken_()
{
	if(!isPunctuator(currentChar_()) && iswgraph(currentChar_())) {
		if(isDigit(currentChar_())) {
			return classifyNumber_();
		} else {
			processCurrentChar_();
			while(!isPunctuator(currentChar_()) && iswgraph(currentChar_())) {
				processCurrentChar_();
			}
			return classifyKeyword_();
		}
	} else {
		switch(currentChar_())
		{
		case L'!' :
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::LOGIC_NOTEQ;
			} else {
				return Token::LOGIC_NOT;
			}

		case L'"' :
			return processString_();

		case L'%' :
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_MOD;
			} else {
				return Token::ARITH_MOD;
			}

		case L'&' :
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_AND;
			} else if (currentChar_() == L'&') {
				processCurrentChar_();
				return Token::LOGIC_AND;
			} else {
				return Token::BIT_AND;
			}

		case L'(':
			processCurrentChar_();
			return Token::LEFTPAREN;

		case L')':
			processCurrentChar_();
			return Token::RIGHTPAREN;

		case L'*':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_MUL;
			} else {
				return Token::ARITH_MUL;
			}

		case L'+':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_ADD;
			} else if (currentChar_() == L'+') {
				processCurrentChar_();
				return Token::ARITH_INC;
			} else {
				return Token::ARITH_ADD;
			}

		case L',':
			processCurrentChar_();
			return Token::COMMA;

		case L'-':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_SUB;
			} else if (currentChar_() == L'-') {
				processCurrentChar_();
				return Token::ARITH_DEC;
			} else {
				return Token::ARITH_SUB;
			}

		case L'.':
			if (isDigit(lookForward_(1))) {
				// If the following character is digit then consider this token as floating point number (ex .142f)
				return classifyNumber_();
			} else {				
				processCurrentChar_();
				return Token::DOT;
			}

		case L'/':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_DIV;
			} else {
				return Token::ARITH_DIV;
			}

		case L':':
			processCurrentChar_();
			return Token::COLON;

		case L';':
			processCurrentChar_();
			return Token::SEMICOLON;

		case L'<':
			processCurrentChar_();
			if (currentChar_() == L'<') {
				processCurrentChar_();
				if (currentChar_() == L'=') {
					processCurrentChar_();
					return Token::ASSIGN_SL;
				} else {
					return Token::BIT_SL;
				}
			} else if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::LOGIC_LE;
			} else {
				return Token::LOGIC_LESS;
			}

		case L'=':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::LOGIC_EQ;
			} else {
				return Token::ASSIGN;
			}

		case L'>':
			processCurrentChar_();
			if (currentChar_() == L'>') {
				processCurrentChar_();
				if (currentChar_() == L'=') {
					processCurrentChar_();
					return Token::ASSIGN_SR;
				} else {
					return Token::BIT_SR;
				}
			} else if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::LOGIC_GE;
			} else {
				return Token::LOGIC_GREATER;
			}

		case L'?':
			processCurrentChar_();
			return Token::LOGIC_IF;
		
		case L'[':
			processCurrentChar_();
			return Token::LEFTBRACKET;

		case L']':
			processCurrentChar_();
			return Token::RIGHTBRACKET;

		case L'^':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_XOR;
			} else {
				return Token::BIT_XOR;
			}

		case L'{':
			processCurrentChar_();
			return Token::LEFTBRACE;

		case L'|':
			processCurrentChar_();
			if (currentChar_() == L'=') {
				processCurrentChar_();
				return Token::ASSIGN_OR;
			} else if (currentChar_() == L'|') {
				processCurrentChar_();
				return Token::LOGIC_OR;
			} else {
				return Token::BIT_OR;
			}

		case L'}':
			processCurrentChar_();
			return Token::RIGHTBRACE;

		case L'~':
			processCurrentChar_();
			return Token::BIT_NOT;

		case 0:
			return Token::END;

		default:
			throw Error(L"%d: unexpected character", currentLine_);
		};
	}
}

Token::Type Scanner::classifyKeyword_()
{
	// Currently, the beginning of integer casted keyword Token enumeration is null
	// If the beginning keyword enumeration is changed, then this routine should be also modified
	for (uint32_t i = Token::KEYWORD_NULL; i < Token::ERR; i++) {
		if(currentLexeme_ == Token::lexeme(static_cast<Token::Type>(i)))
			return static_cast<Token::Type>(i);
	}
	return Token::IDENTIFIER;
}

Token::Type Scanner::classifyNumber_()
{
	if (currentChar_() == L'+' || currentChar_() == L'-') {
		processCurrentChar_();
	}
	if (currentChar_() == L'.') { // float
		return scanFloat_(0);
	} else if (currentChar_() == L'0') {
		processCurrentChar_();
		if (currentChar_() == L'x') { // hexadecimal
			processCurrentChar_();
			if (!isHexadecimal(currentChar_())) {
				throw Error(L"%d: malformed number '%s'", currentLine_, currentLexeme_.c_str());
			}
			while (isHexadecimal(currentChar_())) {
				processCurrentChar_();
			}
			return Token::LITERAL_HEX;
		} else if (currentChar_() == L'.' || isDigit(currentChar_())) {
			return scanFloat_(0);
		} else {
			return Token::LITERAL_INT;
		}
	} else { // decimal
		while(isDigit(currentChar_())) {
			processCurrentChar_();
		}
		if (currentChar_() == L'.' || currentChar_() == L'e' || currentChar_() == L'E') {
			return scanFloat_(3);
		} else {
			return Token::LITERAL_INT;
		}
	}
}

Token::Type Scanner::scanFloat_(const uint32_t entryState)
{
	uint32_t state = entryState;
	assert(entryState == 0 || entryState == 3);

	for (;;) {
		switch (state) {
		case 0:
			if (currentChar_() == L'.') {
				state = 1;
			} else {
				state = 3;
			}
			processCurrentChar_();
			break;
		case 1:
			if (isDigit(currentChar_())) {
				processCurrentChar_();
				state = 2;
			} else {
				state = 6; // error
			}
		case 2:
			while (isDigit(currentChar_())) {
				processCurrentChar_();
			}
			if (currentChar_() == L'e' || currentChar_() == L'E') {
				processCurrentChar_();
				state = 4;
			} else if (currentChar_() == L'f' || currentChar_() == L'F') {
				processCurrentChar_();
				return Token::LITERAL_FLOAT;
			} else {
				return Token::LITERAL_FLOAT;
			}		
			break;
		case 3:
			while (isDigit(currentChar_())) {
				processCurrentChar_();
			}
			if (currentChar_() == L'.') {
				processCurrentChar_();
				state = 2;
			} else if (currentChar_() == L'e' || currentChar_() == L'E') {
				processCurrentChar_();
				state = 4;
			} else {
				state = 6; // error
			}
			break;
		case 4:
			if (currentChar_() == L'+' || currentChar_() == L'-') {
				processCurrentChar_();
			}
			if (isDigit(currentChar_())) {
				processCurrentChar_();
				state = 5;
			} else {
				state = 6; // error
			}
			break;
		case 5:
			while (isDigit(currentChar_())) {
				processCurrentChar_();
			}
			if (currentChar_() == L'f' || currentChar_() == L'F') {
				processCurrentChar_();
			}
			return Token::LITERAL_FLOAT;
		case 6:
			throw Error(L"%d: malformed number '%s'", currentLine_, currentLexeme_.c_str());
		}
	}
	assert(false);
	return Token::ERR; // This statement should not be hit
}


bool Scanner::processEscapeSequence_()
{
	assert(currentChar_() == L'\\');

	skipCurrentChar_();

	switch (currentChar_()) {
	case L'n':
		replaceCurrentChar_(L'\n');
		return true;
	case L't':
		replaceCurrentChar_(L'\t');
		return true;
	case L'\\':
		replaceCurrentChar_(L'\\');
		return true;
	case L'\'':
		replaceCurrentChar_(L'\'');
		return true;
	case L'\"':
		replaceCurrentChar_(L'\\');
		return true;
	case L'x':
	{
		// NOTE : I'm not confident about this routine is correct or not,
		//        so this routine might need to be checked outside of windows platform
		skipCurrentChar_(); 
		
		if (isHexadecimal(currentChar_())) {
			wstring codeNumber;
            uint32_t character; // numeric representation of a unicode character
			do {
				codeNumber.push_back(currentChar_());
				skipCurrentChar_();
			} while (isHexadecimal(currentChar_()));

#pragma warning(push)
#pragma warning(disable:4996)
			swscanf(codeNumber.c_str(), L"%x", &character);
#pragma warning(pop)
			currentLexeme_.push_back(static_cast<wchar_t>(character));
			return true;
		} else {			
			return false;
		}
	}

	default: return false;
	}
}

Token::Type Scanner::processString_()
{
	skipCurrentChar_();

	for (;;) {
		switch (currentChar_()) {
		case L'"':
			skipCurrentChar_();
			return Token::LITERAL_STRING;

		case L'\\':
			if (!processEscapeSequence_()) {
				throw Error(L"%d: illegal escape sequence", currentLine_);
			}
			break;

		case L'\n':
		case 0:
			skipCurrentChar_();
			throw Error(L"%d: missing closing quote", currentLine_);

		default:
			processCurrentChar_();
		}
	}
}

} // namespace "cmm"