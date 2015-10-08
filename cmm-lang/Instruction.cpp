#include "StdAfx.h"
#include "Instruction.h"

#include <string>

namespace cmm
{

const std::wstring Instruction::name[] = {
	L"ASSIGN",
	L"GETCONST",
	L"GETGLOBAL",
	L"GETUPVAL",
	L"GETTABLE",
	L"SETGLOBAL",
	L"SETUPVAL",
	L"SETTABLE",
	L"NEWTABLE",
	L"NEWARRAY",
	L"NEWFUNC",
	L"ADD",
	L"SUB",
	L"MUL",
	L"DIV",
	L"MOD",
	L"UNM",
	L"BITNOT",
	L"BITAND",
	L"BITOR",
	L"BITXOR",
	L"SL",
	L"SR",
	L"NOT",
	L"EQ",
	L"NOTEQ",
	L"LT",
	L"LE",
	L"JUMP",
	L"BRANCH",
	L"BRANCHNOT",
	L"CALL",
	L"RETURN",
	L"YIELD"
};

const Instruction::Type Instruction::type[] = {
	TWO_OP,     // ASSIGN
	TWO_OP,     // GETCONST
	TWO_OP,     // GETGLOBAL
	THREE_OP,   // GETUPVAL
	THREE_OP,   // GETTABLE
	TWO_OP,     // SETGLOBAL
	THREE_OP,   // SETUPVAL
	THREE_OP,   // SETTABLE
	ONE_OP,     // NEWTABLE
	ONE_OP,     // NEWARRAY
	TWO_OP,     // NEWFUNC
	THREE_OP,   // ADD
	THREE_OP,   // SUB
	THREE_OP,   // MUL
	THREE_OP,   // DIV
	THREE_OP,   // MOD
	TWO_OP,     // UNM
	TWO_OP,     // BITNOT
	THREE_OP,   // BITAND
	THREE_OP,   // BITOR
	THREE_OP,   // BITXOR
	THREE_OP,   // SL
	THREE_OP,   // SR
	TWO_OP,     // NOT
	THREE_OP,   // EQ
	THREE_OP,   // NOTEQ
	THREE_OP,   // LT
	THREE_OP,   // LE
	ONE_OP,     // JUMP
	TWO_OP,     // BRANCH
	TWO_OP,     // BRANCHNOT
	THREE_OP,   // CALL
	TWO_OP,     // RETURN
	TWO_OP,     // YIELD
};

} // namespace "cmm"