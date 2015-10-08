#ifndef CODE_PRINTER_H
#define CODE_PRINTER_H

#include <cstdint>
#include <string>

namespace cmm
{

class Prototype;
struct Instruction;
struct Variable;

class CodePrinter
{
public:
	explicit          CodePrinter();
	                  ~CodePrinter();

	std::wstring&     print(const Prototype& prototype);

private:
	                  CodePrinter(const CodePrinter&);
	const CodePrinter operator=(const CodePrinter&);

	void              appendPrototype_(const Prototype& prototype, const uint32_t prototypeNum);
	void              appendPrototypeInfo_(const Prototype& prototype, const uint32_t prototypeNum);
	void              appendPrototypeEnd_();
	void              appendCode_(const Instruction& inst, const uint32_t offset);
	void              appendConstant_(const Variable& constant, const uint32_t constNum);
	void              appendIndention_();
	void              append_(const wchar_t format[], ...);

	std::wstring      output_;
	uint32_t          indentionLevel_;
};

} // The end of namespace "cmm"

#endif