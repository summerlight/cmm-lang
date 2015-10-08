#ifndef SCANNER_H
#define SCANNER_H

#include <cstdint>
#include <string>

#include "Token.h"

namespace cmm
{

class Error;

constexpr uint32_t SCANNER_TAP_SIZE = 8;

class Scanner
{
public:
	explicit          Scanner();
					  ~Scanner();
	                  Scanner(const Scanner&) = delete;
	const Scanner&    operator=(const Scanner&) = delete;

	bool			  load(const wchar_t code[]);
	bool			  load(const wchar_t code[], const uint32_t codeSize);
	Token			  scan();

private:
	Token::Type       scanToken_();
	Token::Type       classifyKeyword_();
	Token::Type       classifyNumber_();
	Token::Type       scanFloat_(const uint32_t entryState);

	Token::Type       processString_();
	bool              processEscapeSequence_();

	void              processCurrentChar_();
	void              skipCurrentChar_();
	void              replaceCurrentChar_(const wchar_t replacement);

	void              skipBlankAndComment_();

	wchar_t           currentChar_();
	wchar_t           lookForward_(uint32_t n);

	const wchar_t     *code_;
	uint32_t          codeSize_;
	std::wstring      currentLexeme_;
	uint32_t          currentOffset_;
	uint32_t          currentLine_;
	uint32_t          currentCol_;
	bool              isScanning_;
};

} // namespace"cmm"

#endif