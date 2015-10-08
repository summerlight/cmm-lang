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
	explicit           CodePrinter();
	                   ~CodePrinter();
                       CodePrinter(const CodePrinter&) = delete;
    const CodePrinter& operator=(const CodePrinter&) = delete;

	std::wstring&      print(const Prototype& prototype);
                      
private:              
	void               appendPrototype_(const Prototype& prototype, uint32_t prototypeNum);
	void               appendPrototypeInfo_(const Prototype& prototype, uint32_t prototypeNum);
	void               appendPrototypeEnd_();
	void               appendCode_(const Instruction& inst, uint32_t offset);
	void               appendConstant_(const Variable& constant, uint32_t constNum);
	void               appendIndention_();
	void               append_(const wchar_t format[], ...);
                      
	std::wstring       output_;
	uint32_t           indentionLevel_;
};

} // namespace "cmm"

#endif