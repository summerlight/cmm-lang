#ifndef SCANNER_H
#define SCANNER_H

#include <cstdint>
#include <string>

#include "Token.h"

namespace cmm
{

class Error;

const uint32_t SCANNER_TAP_SIZE = 8;

class Scanner
{
public:
	explicit                Scanner();
							~Scanner();

	const bool				load(const wchar_t code[]);
	const bool				load(const wchar_t code[], const uint32_t codeSize);
	const Token				scan();

private:
	                        Scanner(const Scanner&);
	const Scanner&          operator=(const Scanner&);

	const Token::Type       scanToken_();
	const Token::Type       classifyKeyword_();
	const Token::Type       classifyNumber_();
	const Token::Type       scanFloat_(const uint32_t entryState);

	const Token::Type       processString_();
	const bool              processEscapeSequence_();

	void                    processCurrentChar_();
	void                    skipCurrentChar_();
	void                    replaceCurrentChar_(const wchar_t replacement);

	void                    skipBlankAndComment_();

	const wchar_t           currentChar_();
	const wchar_t           lookForward_(const uint32_t n);

	const wchar_t           *code_;
	uint32_t                codeSize_;
	std::wstring            currentLexeme_;
	uint32_t                currentOffset_;
	uint32_t                currentLine_;
	uint32_t                currentCol_;
	bool                    isScanning_;
};

} // The end of namespace "cmm"

#endif