#include "StdAfx.h"
#include "CodePrinter.h"

#include <cstdarg>
#include <cassert>
#include <cstdint>
#include <string>

#include "Prototype.h"
#include "Instruction.h"
#include "Memory.h"

namespace cmm
{

CodePrinter::CodePrinter()
: indentionLevel_(0)
{
}

CodePrinter::~CodePrinter()
{
}

std::wstring& CodePrinter::print(const Prototype& prototype)
{
	appendPrototype_(prototype, 0);

	return output_;
}

void CodePrinter::appendPrototype_(const Prototype& prototype, uint32_t prototypeNum)
{
	appendPrototypeInfo_(prototype, prototypeNum);
	
	for (uint32_t i = 0; i < prototype.numConstant(); i++) {
		appendConstant_(prototype.constant(i), i);
	}

	append_(L"\n");

	for (uint32_t i = 0; i < prototype.numInstruction(); i++) {
		appendCode_(prototype.instruction(i), i);
	}

	append_(L"\n");

	for (uint32_t i = 0; i < prototype.numPrototype(); i++) {
		appendPrototype_(*prototype.localPrototype(i), i);
		append_(L"\n");
	}

	appendPrototypeEnd_();
}

void CodePrinter::appendPrototypeInfo_(const Prototype& prototype, uint32_t prototypeNum)
{
	appendIndention_();
	append_(L"; function [%02d] definition\n", prototypeNum);
	appendIndention_();
	append_(L"; function level: %d, argument: %d, local size: %d\n",
	        prototype.functionLevel(), prototype.numArgs(), prototype.localSize());
	appendIndention_();
	append_(L"function[%02d] (%d, %d, %d)\n",
	        prototypeNum, prototype.functionLevel(), prototype.numArgs(), prototype.localSize());
	appendIndention_();
	append_(L"{\n");
	indentionLevel_++;
}

void CodePrinter::appendPrototypeEnd_()
{
	indentionLevel_--;
	appendIndention_();
	append_(L"}\n");
}

void CodePrinter::appendCode_(const Instruction& inst, uint32_t offset)
{
	appendIndention_();
	append_(L"code[%02d] = ", offset);		

	switch (Instruction::type[inst.opcode]) {
	case Instruction::ONE_OP:
		append_(L"%-10s %d", Instruction::name[inst.opcode].c_str(), inst.operand1);
		break;
	case Instruction::TWO_OP:
		append_(L"%-10s %d, %d", Instruction::name[inst.opcode].c_str(), inst.operand1, inst.operand2);
		break;
	case Instruction::THREE_OP:
		append_(L"%-10s %d, %d, %d", Instruction::name[inst.opcode].c_str(), inst.operand1, inst.operand2, inst.operand3);
		break;
	}

	append_(L"\n");
}

void CodePrinter::appendConstant_(const Variable& constant, uint32_t constNum)
{
	appendIndention_();
	append_(L"const[%02d] = ", constNum);

	switch (constant.t) {
	case TypeNull:   append_(L"null"); break;
	case TypeInt:    append_(L"%d", constant.v.i); break;
	case TypeFloat:  append_(L"%f", constant.v.f); break;
	case TypeString: {
		String* str = static_cast<String*>(constant.v.obj);
		append_(L"\"%s\"", str->value().c_str());
		break;
	}
	default:         assert(false); // there is no other constant type
	}

	append_(L"\n");
}

void CodePrinter::appendIndention_()
{
	for (uint32_t i = 0; i < indentionLevel_; i++) {
		append_(L"    ");
	}
}

void CodePrinter::append_(const wchar_t format[], ...)
{
	va_list args;
	va_start(args, format);
	wchar_t buffer[256];

	vswprintf(buffer, 256, format, args);

	output_.append(buffer);
}

} // namespace "cmm"