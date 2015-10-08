#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>

namespace cmm
{

struct Instruction
{
	enum Opcode {
		ASSIGN,     // A B      R(A) = R(B)
		GETCONST,   // A B      R(A) = C(B)
		GETGLOBAL,  // A B      R(A) = G(C(B))
		GETUPVAL,   // A B C    R(A) = UP(C)(B)
		GETTABLE,   // A B C    R(A) = R(B)[R(C)]
		SETGLOBAL,  // A B      G(C(A)) = R(B)
		SETUPVAL,   // A B C    UP(C)(A) = R(B)
		SETTABLE,   // A B C    R(A)[R(C)] = R(B)
		NEWTABLE,   // A        R(A) = new table
		NEWARRAY,   // A        R(A) = new array
		NEWFUNC,    // A B      R(A) = new func with prototype (B)
		ADD,        // A B C    R(A) = R(B) + R(C), String concatenation if both R(B) and R(C) are string
		SUB,        // A B C    R(A) = R(B) - R(C)
		MUL,        // A B C    R(A) = R(B) * R(C)
		DIV,        // A B C    R(A) = R(B) / R(C)
		MOD,        // A B C    R(A) = R(B) % R(C)
		UNM,        // A B      R(A) = -R(B)
		BITNOT,     // A B      R(A) = ~R(B)
		BITAND,     // A B C    R(A) = R(B) & R(C)
		BITOR,      // A B C    R(A) = R(B) | R(C)
		BITXOR,     // A B C    R(A) = R(B) ^ R(C)
		SL,         // A B C    R(A) = R(B) << R(C)
		SR,         // A B C    R(A) = R(B) >> R(C)
		NOT,        // A B      R(A) = ! R(B)
		EQ,         // A B C    R(A) = R(B) == R(C)
		NOTEQ,      // A B C    R(A) = R(B) != R(C)
		LT,         // A B C    R(A) = R(B) <  R(C)
		LE,         // A B C    R(A) = R(B) <= R(C)
		JUMP,       // A        PC += A
		BRANCH,     // A B      if (R(A)) PC += B
		BRANCHNOT,  // A B      if (!R(A)) PC += B
		CALL,       // A B C    R(A), R(A+1) ... R(A+C-1) = R(A)(R(A+1), R(A+2) ... R(A+B)
		RETURN,     // A B      return R(A), R(A+1), ... R(A+B-1)
		YIELD       // A B      yield R(A), R(A+1), ... R(A+B-1)
	};

	enum Type {
		ONE_OP,
		TWO_OP,
		THREE_OP
	};

	Instruction() {}
	Instruction(const Instruction& inst)
		: opcode(inst.opcode), operand1(inst.operand1), operand2(inst.operand2), operand3(inst.operand3) {}

	Instruction(int32_t opcode, int32_t op1, int32_t op2, int32_t op3)
	: opcode(opcode), operand1(op1), operand2(op2), operand3(op3) {}

	int32_t   opcode;
	int32_t   operand1;
	int32_t   operand2;
	int32_t   operand3;


	static const std::wstring   name[];
	static const Type           type[];
};



} // namespace "cmm"

#endif