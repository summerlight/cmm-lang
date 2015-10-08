#include "StdAfx.h"
#include "CodeGenerator.h"

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <iterator>

#include "AST.h"
#include "ASTVisitor.h"
#include "Prototype.h"
#include "Instruction.h"
#include "Error.h"
#include "Memory.h"

namespace cmm
{

namespace {

#pragma warning (push)
#pragma warning (disable:4996)
int32_t whtoi(const wchar_t lexeme[])
{
	int32_t integer;
	swscanf(lexeme, L"0x%x", &integer);
	return integer;
}

int32_t wtoi(const wchar_t lexeme[])
{
	int32_t integer;
	swscanf(lexeme, L"%d", &integer);
	return integer;
}

float wtof(const wchar_t lexeme[])
{
	float floating;
	swscanf(lexeme, L"%f", &floating);
	return floating;
}
#pragma warning (pop)

} // The end of anonymous namespace for utility functions only used in code generator class



CodeGenerator::CodeGenerator(ObjectManager& objectManager)
: objectManager_(objectManager)
{
}


CodeGenerator::~CodeGenerator()
{
}



Ref<Prototype> CodeGenerator::createPrototype(AST::FunctionDefinition& functionDef)
{
	prototype_ = Ref<Prototype>(new Prototype(&objectManager_));

	safeVisit_(functionDef.arguments.get());
	safeVisit_(functionDef.contents.get());

	appendCode_(Instruction::RETURN, 0, 0);

	substitueLabelToOffset();
	assert(functionDef.functionLevel != UINT32_MAX);
	assert(functionDef.numVariable != UINT32_MAX);
	prototype_->localSize_ = register_.maxSize();
	prototype_->functionLevel_ = functionDef.functionLevel;
	prototype_->numArgs_ = functionDef.arguments->statementList.size();

	return prototype_;
}


bool CodeGenerator::safeVisit_(AST::Base* host)
{
	if (host == nullptr) {
		return false;
	} else {
		host->accept(*this);
		return true;
	}
}


uint32_t CodeGenerator::addConstant_(const Variable& constant)
{
	auto &constTable = prototype_->constants_;
	Variable::StrictEqual isEqual;

	for (uint32_t i = 0; i < constTable.size(); i++) {
		if (isEqual(constTable[i], constant)) {
			return i;
		}
	}

	constTable.push_back(constant);
	return constTable.size() - 1;
}

void CodeGenerator::substitueLabelToOffset()
{
	auto &code = prototype_->code_;

	for (uint32_t offset = 0; offset < code.size(); offset++) {
		if (code[offset].opcode == Instruction::JUMP) {
			assert(labelManager_.getOffset(code[offset].operand1) != UINT32_MAX);
			code[offset].operand1 = labelManager_.getOffset(code[offset].operand1) - offset;
		} else if (code[offset].opcode == Instruction::BRANCH) {
			assert(labelManager_.getOffset(code[offset].operand2) != UINT32_MAX);
			code[offset].operand2 = labelManager_.getOffset(code[offset].operand2) - offset;
		} else if (code[offset].opcode == Instruction::BRANCHNOT) {
			assert(labelManager_.getOffset(code[offset].operand2) != UINT32_MAX);
			code[offset].operand2 = labelManager_.getOffset(code[offset].operand2) - offset;
		}
	}
}


inline void CodeGenerator::appendCode_(const Instruction::Opcode op, const int32_t operand1,
                                       const int32_t operand2, const int32_t operand3)
{
	assert(operand1 != UINT32_MAX && operand2 != UINT32_MAX && operand3 != UINT32_MAX);
	
	if (op == Instruction::ASSIGN && operand1 == operand2) { return; }
	
	auto &code = prototype_->code_;
	code.push_back(Instruction(op, operand1, operand2, operand3));
}

inline uint32_t CodeGenerator::nextOffset_()
{
	auto &code = prototype_->code_;
	return code.size();
}

/*
 * Append unary operation
 *
 * Pre-condition :
 *  - a first sub-node can be any of l-value or r-value
 *  - a first sub-node should have a register offset that contains the result of partial expression
 *
 * Post-condition :
 *  - a register of first sub-node would be freed if it was temporary register
 *  - a newly allocated register will store the result of operation and be returned
 *  - GETCONST 0 and a corresponding unary instruction will be appended to the instruction vector
 */
void CodeGenerator::appendUnaryOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op)
{
	AST::Expression &firstExpr = *unaryExpr.first;

	safeVisit_(unaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);

	register_.deallocate(firstExpr);

	unaryExpr.registerOffset = register_.allocate();
	unaryExpr.flag |= AST::FLAG_TEMP;

	uint32_t constant_0 = addConstant_(Variable(0));
	uint32_t constRegister = register_.allocate();
	appendCode_(Instruction::GETCONST, constRegister, constant_0);
	appendCode_(op, unaryExpr.registerOffset, constRegister, firstExpr.registerOffset);
	register_.deallocate(constRegister);
}

/*
 * Append prefix operation
 *
 * Pre-condition :
 *  - a first sub-node should be any of l-value
 *  - a first sub-node should have STORE flag
 *  - a first sub-node should have a register offset that contains the result of partial expression
 *
 * Post-condition :
 *  - a register of first sub-node would be freed if it was temporary register
 *  - a newly allocated register will store the result of operation and be returned
 *  - GETCONST 1 and a corresponding binary, store instruction will be appended to the instruction vector
 */
void CodeGenerator::appendPrefixOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op)
{
	AST::Expression &firstExpr = *unaryExpr.first;

	safeVisit_(unaryExpr.first.get());
	assert((firstExpr.flag & AST::FLAG_STORE) && (firstExpr.flag & AST::FLAG_LVALUE));
	assert(firstExpr.registerOffset != UINT32_MAX);

	uint32_t constant_1 = addConstant_(Variable(1));
	uint32_t constRegister = register_.allocate();
	appendCode_(Instruction::GETCONST, constRegister, constant_1);

	appendCode_(op, firstExpr.registerOffset, firstExpr.registerOffset, constRegister);
	
	register_.deallocate(constRegister);

	appendStoreOp_(firstExpr, firstExpr);
	register_.deallocate(firstExpr);
	
	uint32_t tempRegister = register_.allocate();
	appendCode_(Instruction::ASSIGN, tempRegister, firstExpr.registerOffset);
		
	unaryExpr.registerOffset = tempRegister;
}

/*
 * Append postfix operation
 *
 * Pre-condition :
 *  - a first sub-node should be any of l-value
 *  - a first sub-node should have STORE flag
 *  - a first sub-node should have a register offset that contains the result of partial expression
 *
 * Post-condition :
 *  - a register of first sub-node would be freed if it was temporary register
 *  - a newly allocated register will store original value
 *  - the result of postfix operation will be stored in the original register
 *  - GETCONST 1 and a corresponding binary, store instruction will be appended to the instruction vector
 */
void CodeGenerator::appendPostfixOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op)
{
	AST::Expression &firstExpr = *unaryExpr.first;

	safeVisit_(unaryExpr.first.get());
	assert((firstExpr.flag & AST::FLAG_STORE) && (firstExpr.flag & AST::FLAG_LVALUE));
	assert(firstExpr.registerOffset != UINT32_MAX);

	uint32_t tempRegister = register_.allocate();
	appendCode_(Instruction::ASSIGN, tempRegister, firstExpr.registerOffset);

	uint32_t constRegister = register_.allocate();
	uint32_t constant_1 = addConstant_(Variable(1));
	appendCode_(Instruction::GETCONST, constRegister, constant_1);

	if (firstExpr.flag & AST::FLAG_TEMP) {
		appendCode_(op, constRegister, firstExpr.registerOffset, constRegister);
		appendStoreOp_(constRegister, firstExpr);
	} else {
		appendCode_(op, firstExpr.registerOffset, firstExpr.registerOffset, constRegister);		
	}

	register_.deallocate(constRegister);
	register_.deallocate(tempRegister);
	register_.deallocate(firstExpr);
	
	unaryExpr.registerOffset = register_.allocate();
	unaryExpr.flag |= AST::FLAG_TEMP;
	appendCode_(Instruction::ASSIGN, unaryExpr.registerOffset, tempRegister);
}


/*
 * Append binary operation
 *
 * Pre-condition :
 *  - both sub-nodes can be any of l-value or r-value
 *  - both sub-nodes should have a register offset that contains the result of partial expression
 *
 * Post-condition :
 *  - registers of both sub-node would be freed if they are temporary registers
 *  - a newly allocated register will store the result of operation value and be returned
 *  - a corresponding binary instruction will be appended to the instruction vector
 */
void CodeGenerator::appendBinaryOp_(AST::BinaryExpr& binaryExpr, Instruction::Opcode op, bool inverted)
{
	AST::Expression &firstExpr = *binaryExpr.first;
	AST::Expression &secondExpr = *binaryExpr.second;

	safeVisit_(binaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);
	
	safeVisit_(binaryExpr.second.get());
	assert(secondExpr.registerOffset != UINT32_MAX);

	register_.deallocate(secondExpr);
	register_.deallocate(firstExpr);		

	binaryExpr.registerOffset = register_.allocate();
	binaryExpr.flag |= AST::FLAG_TEMP;

	if (inverted == true) {
		appendCode_(op, binaryExpr.registerOffset, secondExpr.registerOffset, firstExpr.registerOffset);
	} else {
		appendCode_(op, binaryExpr.registerOffset, firstExpr.registerOffset, secondExpr.registerOffset);
	}
}


/*
 * Append compound assign operation
 *
 * Pre-condition :
 *  - first sub-node can be any of l-value or r-value
 *  - second sub-node should be any of l-value
 *  - first sub-node should have a register offset that contains the result of partial expression
 *  - second sub-node should have a register offset if it is local l-value
 *  - second sub-node should have STORE flag
 *
 * Post-condition :
 *  - registers of both sub-node would be freed if they are temporary registers
 *  - a newly allocated register will store the result of operation and be returned
 *  - a corresponding binary instruction will be appended to the instruction vector
 */
void CodeGenerator::appendAssignOp_(AST::BinaryExpr& binaryExpr, Instruction::Opcode op)
{
	AST::Expression &firstExpr = *binaryExpr.first;
	AST::Expression &secondExpr = *binaryExpr.second;
		
	safeVisit_(binaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);
	
	safeVisit_(binaryExpr.second.get());
	assert(secondExpr.registerOffset != UINT32_MAX || secondExpr.lvalue1 != UINT32_MAX);
	assert((secondExpr.flag & AST::FLAG_STORE) && (secondExpr.flag & AST::FLAG_LVALUE));

	register_.deallocate(secondExpr);
	register_.deallocate(firstExpr);
	binaryExpr.registerOffset = register_.allocate();
	binaryExpr.flag |= AST::FLAG_TEMP;
	
	appendCode_(op, binaryExpr.registerOffset, secondExpr.registerOffset, firstExpr.registerOffset);
	appendStoreOp_(binaryExpr, secondExpr);
}


/*
 * Append assign operation (assign first value to second value)
 *
 * Pre-condition :
 *  - first sub-node can be any of l-value or r-value
 *  - second sub-node should be any of l-value
 *  - first sub-node should have a register offset that contains the result of partial expression
 *  - second sub-node should have a register offset if it is local l-value
 *  - second sub-node should have STORE flag
 *
 * Post-condition :
 *  - registers of both sub-node would be freed if they are temporary registers
 *  - a newly allocated register will store the result of operation value and be returned
 *  - l-value information of second sub-node will be copied to assign operator node
 *  - a corresponding binary instruction will be appended to the instruction vector
 */
void CodeGenerator::appendAssignOp_(AST::BinaryExpr& binaryExpr)
{
	AST::Expression &firstExpr = *binaryExpr.first;
	AST::Expression &secondExpr = *binaryExpr.second;

	safeVisit_(binaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);
	
	safeVisit_(binaryExpr.second.get());
	assert(secondExpr.registerOffset != UINT32_MAX || secondExpr.lvalue1 != UINT32_MAX);	
	assert((secondExpr.flag & AST::FLAG_LVALUE) && (secondExpr.flag & AST::FLAG_STORE));

	register_.deallocate(secondExpr);
	register_.deallocate(firstExpr);
	
	binaryExpr.registerOffset = register_.allocate();
	binaryExpr.flag |= AST::FLAG_TEMP;		
	appendStoreOp_(firstExpr, secondExpr);
	appendCode_(Instruction::ASSIGN, binaryExpr.registerOffset, firstExpr.registerOffset);
}

/*
 * Append store operation
 *
 * Pre-condition :
 *  - both input node should have the register offset
 *
 * Post-condition :
 *  - a corresponding store instruction will be appended to the instruction vector
 */
inline void CodeGenerator::appendStoreOp_(const uint32_t valueRegister, const AST::Expression& dest)
{
	assert((dest.registerOffset != UINT32_MAX) || (dest.lvalue1 != UINT32_MAX));
	assert(valueRegister != UINT32_MAX);

	if (dest.flag & AST::FLAG_TABLE) {
		appendCode_(Instruction::SETTABLE, dest.lvalue1, valueRegister, dest.lvalue2);
	} else if (dest.flag & AST::FLAG_UPVALUE) {
		appendCode_(Instruction::SETUPVAL, dest.lvalue2, valueRegister, dest.lvalue1);
	} else if (dest.flag & AST::FLAG_GLOBAL) {
		appendCode_(Instruction::SETGLOBAL, dest.lvalue1, valueRegister);
	} else {
		appendCode_(Instruction::ASSIGN, dest.registerOffset, valueRegister);
	}
}

inline void CodeGenerator::appendStoreOp_(const AST::Expression& value, const AST::Expression& dest)
{
	appendStoreOp_(value.registerOffset, dest);
}


/*
 * The below code is psuedo assembly code for short-cut binary AND/OR operation
 *
 *  - a && b             |      - a || b
 *                       |
 * t = a()               |     t = a()
 * BRANCHNOT t false     |     BRANCH t true
 * t = b()               |     t = b()
 * BRANCHNOT t false     |     BRANCH t true
 * t = true              |     t = false
 * JUMP end              |     JUMP end
 * result : t = false    |     result : t = true
 * end : return t        |     end : return t
 */
void CodeGenerator::appendShortCutLogic_(AST::BinaryExpr& binaryExpr)
{
	AST::Expression &firstExpr = *binaryExpr.first;
	AST::Expression &secondExpr = *binaryExpr.second;

	uint32_t constant_0 = addConstant_(Variable(0));
	uint32_t constant_1 = addConstant_(Variable(1));	

	assert(binaryExpr.op == AST::BinaryExpr::LOGIC_AND || binaryExpr.op == AST::BinaryExpr::LOGIC_OR);

	Instruction::Opcode branchInstruction;
	uint32_t firstConstant, secondConstant;
	
	if (binaryExpr.op == AST::BinaryExpr::LOGIC_AND) {
		branchInstruction = Instruction::BRANCHNOT;
		firstConstant = constant_1;
		secondConstant = constant_0;
	} else { 
		branchInstruction = Instruction::BRANCH;
		firstConstant = constant_0;
		secondConstant = constant_1;
	}

	uint32_t resultLabel = labelManager_.newLabel();

	safeVisit_(binaryExpr.first.get());
	appendCode_(branchInstruction, firstExpr.registerOffset, resultLabel);
	register_.deallocate(firstExpr);
	// evaluate second expression, if the result is false then jump to value label
	safeVisit_(binaryExpr.second.get());
	appendCode_(branchInstruction, secondExpr.registerOffset, resultLabel);
	register_.deallocate(secondExpr);
	
	binaryExpr.registerOffset = register_.allocate();
	
	// load true/false value to register
	appendCode_(Instruction::GETCONST, binaryExpr.registerOffset, firstConstant);

	uint32_t endLabel = labelManager_.newLabel();
	appendCode_(Instruction::JUMP, endLabel);

	// result: load false/true value to register
	labelManager_.setOffset(resultLabel, nextOffset_());
	appendCode_(Instruction::GETCONST, binaryExpr.registerOffset, secondConstant);

	// end:
	labelManager_.setOffset(endLabel, nextOffset_());
}


/*
 * Append table load operation
 *
 * Pre-condition :
 *  - binary node should be table index node
 *  - both sub-nodes should have a register offset that contains the result of partial expression
 *
 * Post-condition :
 *  - a corresponding table load instruction will be appended to the instruction vector
 */
void CodeGenerator::appendTableLoadOp_(AST::BinaryExpr& binaryExpr)
{
	AST::Expression &firstExpr = *binaryExpr.first;
	AST::Expression &secondExpr = *binaryExpr.second;

	assert(binaryExpr.op == AST::BinaryExpr::INDEX);
	
	if (!(binaryExpr.flag & AST::FLAG_NOLOAD)) {
		binaryExpr.registerOffset = register_.allocate();
		binaryExpr.flag |= AST::FLAG_TEMP;
	}

	safeVisit_(binaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);
	
	safeVisit_(binaryExpr.second.get());
	assert(secondExpr.registerOffset != UINT32_MAX);

	if (!(binaryExpr.flag & AST::FLAG_NOLOAD)) {
		appendCode_(Instruction::GETTABLE, binaryExpr.registerOffset, firstExpr.registerOffset, secondExpr.registerOffset);
	}

	if (binaryExpr.flag & AST::FLAG_STORE) {
		binaryExpr.lvalue1 = firstExpr.registerOffset;
		binaryExpr.lvalue2 = secondExpr.registerOffset;
		binaryExpr.flag |= AST::FLAG_TEMPTABLE;
	} else {
		register_.deallocate(secondExpr);
		register_.deallocate(firstExpr);
	}
}


void CodeGenerator::visit(AST::StmtSequence& stmtSequence)
{
	AST::StmtSequence::StatementVector &stmtList = stmtSequence.statementList;

	std::for_each(stmtList.begin(), stmtList.end(),
		[this](decltype(*stmtList.begin()) i) { safeVisit_(i.get()); } );
}


void CodeGenerator::visit(AST::TableInitializer& tableInit)
{
	AST::Expression& keyExpr = *tableInit.key;
	AST::Expression& valueExpr = *tableInit.value;

	safeVisit_(tableInit.key.get());
	assert(keyExpr.registerOffset != UINT32_MAX);

	safeVisit_(tableInit.value.get());
	assert(valueExpr.registerOffset != UINT32_MAX);

	appendCode_(Instruction::SETTABLE, tableInit.tableOffset, valueExpr.registerOffset, keyExpr.registerOffset);

	register_.deallocate(valueExpr);
	register_.deallocate(keyExpr);
}


void CodeGenerator::visit(AST::FunctionDefinition& functionDef)
{
	CodeGenerator codeGen(objectManager_);

	functionDef.functionNum = prototype_->localPrototypes_.size();
	Ref<Prototype> localPrototype = codeGen.createPrototype(functionDef);
	prototype_->localPrototypes_.push_back(localPrototype);
}


void CodeGenerator::visit(AST::CompoundStmt& compoundStmt)
{
	safeVisit_(compoundStmt.contents.get());
}

/*
 * Below is psuedo assembly code of for statement
 *
 *            init()
 * condition: R1 = cond()
 *            BRANCHNOT R1 break
 *            content()
 * continue:  iter()
 *            JUMP condition
 * break:
 *
 */

void CodeGenerator::visit(AST::ForStmt& forStmt)
{
	// allocate new label for continue and break destination
	uint32_t conditionLabel = labelManager_.newLabel();
	forStmt.continueLabel = labelManager_.newLabel();
	forStmt.breakLabel = labelManager_.newLabel();

	// generate byte code for initial statement
	safeVisit_(forStmt.initial.get());

	labelManager_.setOffset(conditionLabel, nextOffset_());
	safeVisit_(forStmt.condition.get());
	assert(forStmt.condition->registerOffset != UINT32_MAX);
	
	// if the condition is satisfied then branch to content (which is right after break instruction)
	appendCode_(Instruction::BRANCHNOT, forStmt.condition->registerOffset, forStmt.breakLabel);
	register_.deallocate(*forStmt.condition);
		
	safeVisit_(forStmt.contents.get());

	labelManager_.setOffset(forStmt.continueLabel, nextOffset_());
	safeVisit_(forStmt.iteration.get());
	
	// jump to condition check
	appendCode_(Instruction::JUMP, conditionLabel);

	labelManager_.setOffset(forStmt.breakLabel, nextOffset_());
}


/*
 * Below is psuedo assembly code of while statement
 *
 * continue:  R1 = cond()
 *            BRANCHNOT R1 break
 *            content()
 *            JUMP continue
 * break:
 */

void CodeGenerator::visit(AST::WhileStmt& whileStmt)
{
	// allocate new label for continue and break destination
	whileStmt.continueLabel = labelManager_.newLabel();
	whileStmt.breakLabel = labelManager_.newLabel();

	labelManager_.setOffset(whileStmt.continueLabel, nextOffset_());

	safeVisit_(whileStmt.condition.get());
	assert(whileStmt.condition->registerOffset != UINT32_MAX);

	// if the condition is satisfied then branch to content (which is right after break instruction)
	appendCode_(Instruction::BRANCH, whileStmt.condition->registerOffset, whileStmt.breakLabel);
	register_.deallocate(*whileStmt.condition);

	safeVisit_(whileStmt.contents.get());

	// jump to continue label
	appendCode_(Instruction::JUMP, whileStmt.continueLabel);

	labelManager_.setOffset(whileStmt.breakLabel, nextOffset_());
}

/*
 * Below is psuedo assembly code of do-while statement 
 *
 * begin:      content()
 * continue:   R1 = cond()
 *             BRANCH R1 begin
 * break:
 */

void CodeGenerator::visit(AST::DoWhileStmt& doWhileStmt)
{
	// allocate new label for begin and continue, break destination
	uint32_t beginLabel = labelManager_.newLabel();
	doWhileStmt.continueLabel = labelManager_.newLabel();
	doWhileStmt.breakLabel = labelManager_.newLabel();

	labelManager_.setOffset(beginLabel, nextOffset_());
	safeVisit_(doWhileStmt.contents.get());

	// if the condition is satisfied then branch to content (which is right after break instruction)
	labelManager_.setOffset(doWhileStmt.continueLabel, nextOffset_());
	safeVisit_(doWhileStmt.condition.get());
	assert(doWhileStmt.condition->registerOffset != UINT32_MAX);
	appendCode_(Instruction::BRANCH, doWhileStmt.condition->registerOffset, beginLabel);
	register_.deallocate(*doWhileStmt.condition);

	labelManager_.setOffset(doWhileStmt.breakLabel, nextOffset_());
}

/*
 * Below is psuedo assembly code of if-else statement
 *
 *        R1 = cond()
 *        BRANCHNOT R1 else
 * if:    if()
 *        JUMP end
 * else:  else()
 * end:
 */

void CodeGenerator::visit(AST::IfElseStmt& ifElseStmt)
{
	uint32_t elseLabel = labelManager_.newLabel();
	uint32_t endLabel = labelManager_.newLabel();

	safeVisit_(ifElseStmt.condition.get());
	assert(ifElseStmt.condition->registerOffset != UINT32_MAX);

	// If the condition is not satisfied then branch to else label
	appendCode_(Instruction::BRANCHNOT, ifElseStmt.condition->registerOffset, elseLabel);
	register_.deallocate(*ifElseStmt.condition);
		
	safeVisit_(ifElseStmt.ifContents.get());

	// when the if-chunk has ended then jump to end label
	appendCode_(Instruction::JUMP, endLabel); // defer update for jump instruction to end label

	// set the offset of else label
	labelManager_.setOffset(elseLabel, nextOffset_());	
	safeVisit_(ifElseStmt.elseContents.get());

	// set the offset of end label
	labelManager_.setOffset(endLabel, nextOffset_());
}

	
void CodeGenerator::visit(AST::ReturnStmt& returnStmt)
{
	safeVisit_(returnStmt.returnExpr.get());
	assert(returnStmt.returnExpr == nullptr || returnStmt.returnExpr->registerOffset != UINT32_MAX);

	Instruction::Opcode type;

	if (returnStmt.type == AST::ReturnStmt::RETURN) {
		type = Instruction::RETURN;
	} else {
		type = Instruction::YIELD;
	}

	if (returnStmt.returnExpr == nullptr) {
		appendCode_(type, 0, 0);
	} else {
		appendCode_(type, returnStmt.returnExpr->registerOffset, 1);
	}
}


void CodeGenerator::visit(AST::JumpStmt& jumpStmt)
{
	AST::LoopStmt &loop = *jumpStmt.correspondingLoop;
	uint32_t jumpLabel = (jumpStmt.type == AST::JumpStmt::BREAK) ? loop.breakLabel : loop.continueLabel; 

	appendCode_(Instruction::JUMP, jumpLabel);
}


void CodeGenerator::visit(AST::VariableStmt& variableStmt)
{
	variableStmt.registerOffset = register_.allocate();
		
	safeVisit_(variableStmt.init.get());

	if (variableStmt.init != nullptr) {
		AST::Expression &initializer = *variableStmt.init;
		assert(initializer.registerOffset != UINT32_MAX);
		appendCode_(Instruction::ASSIGN, variableStmt.registerOffset, initializer.registerOffset);
		register_.deallocate(initializer);
	}
}


void CodeGenerator::visit(AST::ExpressionStmt& expressionStmt)
{
	AST::Expression &expr = *expressionStmt.expression;
	safeVisit_(expressionStmt.expression.get());
	register_.deallocate(expr);
}


void CodeGenerator::visit(AST::UnaryExpr& unaryExpr)
{
	switch (unaryExpr.op) {
	case AST::UnaryExpr::PLUS:        appendUnaryOp_(unaryExpr, Instruction::ADD); break;
	case AST::UnaryExpr::MINUS:       appendUnaryOp_(unaryExpr, Instruction::SUB); break;
	case AST::UnaryExpr::PREFIX_INC:  appendPrefixOp_(unaryExpr, Instruction::ADD); break; 
	case AST::UnaryExpr::PREFIX_DEC:  appendPrefixOp_(unaryExpr, Instruction::SUB); break; 
	case AST::UnaryExpr::POSTFIX_INC: appendPostfixOp_(unaryExpr, Instruction::ADD); break;
	case AST::UnaryExpr::POSTFIX_DEC: appendPostfixOp_(unaryExpr, Instruction::SUB); break;
	case AST::UnaryExpr::BIT_NOT:
		unaryExpr.registerOffset = register_.allocate();
		appendCode_(Instruction::BITNOT, unaryExpr.registerOffset, unaryExpr.first->registerOffset);
		return;
	case AST::UnaryExpr::LOGIC_NOT:
		unaryExpr.registerOffset = register_.allocate();
		appendCode_(Instruction::NOT, unaryExpr.registerOffset, unaryExpr.first->registerOffset);
		return;
	}
}


void CodeGenerator::visit(AST::BinaryExpr& binaryExpr)
{
	if (binaryExpr.op == AST::BinaryExpr::LOGIC_AND || binaryExpr.op == AST::BinaryExpr::LOGIC_OR) {
		appendShortCutLogic_(binaryExpr);
		return;
	} else if (binaryExpr.op == AST::BinaryExpr::INDEX) {
		appendTableLoadOp_(binaryExpr);
		return;
	}

	switch (binaryExpr.op) {
		case AST::BinaryExpr::ARITH_ADD:     appendBinaryOp_(binaryExpr, Instruction::ADD); break;
		case AST::BinaryExpr::ARITH_SUB:     appendBinaryOp_(binaryExpr, Instruction::SUB); break;
		case AST::BinaryExpr::ARITH_MUL:     appendBinaryOp_(binaryExpr, Instruction::MUL); break;
		case AST::BinaryExpr::ARITH_DIV:     appendBinaryOp_(binaryExpr, Instruction::DIV); break;
		case AST::BinaryExpr::ARITH_MOD:     appendBinaryOp_(binaryExpr, Instruction::MOD); break;
		case AST::BinaryExpr::BIT_AND:       appendBinaryOp_(binaryExpr, Instruction::BITAND); break;
		case AST::BinaryExpr::BIT_OR:        appendBinaryOp_(binaryExpr, Instruction::BITOR); break;
		case AST::BinaryExpr::BIT_XOR:       appendBinaryOp_(binaryExpr, Instruction::BITXOR); break;
		case AST::BinaryExpr::BIT_SL:        appendBinaryOp_(binaryExpr, Instruction::SL); break;
		case AST::BinaryExpr::BIT_SR:        appendBinaryOp_(binaryExpr, Instruction::SR); break;
		case AST::BinaryExpr::ASSIGN:        appendAssignOp_(binaryExpr); break;
		case AST::BinaryExpr::ASSIGN_ADD:    appendAssignOp_(binaryExpr, Instruction::ADD); break;
		case AST::BinaryExpr::ASSIGN_SUB:    appendAssignOp_(binaryExpr, Instruction::SUB); break;
		case AST::BinaryExpr::ASSIGN_MUL:    appendAssignOp_(binaryExpr, Instruction::MUL); break;
		case AST::BinaryExpr::ASSIGN_DIV:    appendAssignOp_(binaryExpr, Instruction::DIV); break;
		case AST::BinaryExpr::ASSIGN_MOD:    appendAssignOp_(binaryExpr, Instruction::MOD); break;
		case AST::BinaryExpr::ASSIGN_SL:     appendAssignOp_(binaryExpr, Instruction::SL); break;
		case AST::BinaryExpr::ASSIGN_SR:     appendAssignOp_(binaryExpr, Instruction::SR); break;
		case AST::BinaryExpr::ASSIGN_AND:    appendAssignOp_(binaryExpr, Instruction::BITAND); break;
		case AST::BinaryExpr::ASSIGN_OR:     appendAssignOp_(binaryExpr, Instruction::BITOR); break;
		case AST::BinaryExpr::ASSIGN_XOR:    appendAssignOp_(binaryExpr, Instruction::BITXOR); break;
		case AST::BinaryExpr::LOGIC_EQ:      appendBinaryOp_(binaryExpr, Instruction::EQ); break;
		case AST::BinaryExpr::LOGIC_NOTEQ:   appendBinaryOp_(binaryExpr, Instruction::NOTEQ); break;
		case AST::BinaryExpr::LOGIC_GREATER: appendBinaryOp_(binaryExpr, Instruction::LE, true); break;
		case AST::BinaryExpr::LOGIC_GE:      appendBinaryOp_(binaryExpr, Instruction::LT, true); break;
		case AST::BinaryExpr::LOGIC_LESS:    appendBinaryOp_(binaryExpr, Instruction::LT); break;
		case AST::BinaryExpr::LOGIC_LE:      appendBinaryOp_(binaryExpr, Instruction::LE); break;
		default:                             assert(false); break;
	}
}

/*
 * below is pseudo bytecode for trinary conditional expression
 *
 *         R1 = first()
 *         BRANCH R1 third
 *         R1 = second()
 *         JUMP end
 * third:  R1 = third()
 * end:    return R1
 */

void CodeGenerator::visit(AST::TrinaryExpr& trinaryExpr)
{
	AST::Expression &firstExpr = *trinaryExpr.first;
	AST::Expression &secondExpr = *trinaryExpr.second;
	AST::Expression &thirdExpr = *trinaryExpr.third;

	// allocate new label for third expression and end
	uint32_t thirdExprLabel = labelManager_.newLabel();
	uint32_t endLabel = labelManager_.newLabel();

	// evaluate first expression
	safeVisit_(trinaryExpr.first.get());
	assert(firstExpr.registerOffset != UINT32_MAX);

	// if first expression is evaluated as false then jump to third expression
	appendCode_(Instruction::BRANCHNOT, firstExpr.registerOffset, thirdExprLabel);
	register_.deallocate(firstExpr.registerOffset);

	// else evaluate second expression and jump to end label
	safeVisit_(trinaryExpr.second.get());
	assert(secondExpr.registerOffset != UINT32_MAX);
	register_.deallocate(secondExpr.registerOffset);
	appendCode_(Instruction::JUMP, endLabel);

	// evaluate third expression and 
	labelManager_.setOffset(thirdExprLabel, nextOffset_());
	safeVisit_(trinaryExpr.third.get());
	assert(thirdExpr.registerOffset != UINT32_MAX);

	assert(secondExpr.registerOffset == thirdExpr.registerOffset);
	trinaryExpr.registerOffset = secondExpr.registerOffset;

	labelManager_.setOffset(endLabel, nextOffset_());
}


void CodeGenerator::visit(AST::TerminalExpr& terminalExpr)
{
	if (terminalExpr.type == AST::TerminalExpr::IDENTIFIER) {
		AST::VariableStmt &corresponding = *terminalExpr.correspondingVar;

		if (terminalExpr.flag & AST::FLAG_GLOBAL) {
			uint32_t constIndex = addConstant_(Variable(TypeString, new String(terminalExpr.lexeme, &objectManager_)));
			if (!(terminalExpr.flag & AST::FLAG_NOLOAD)) {
				terminalExpr.registerOffset = register_.allocate();
				terminalExpr.flag |= AST::FLAG_TEMP;
				appendCode_(Instruction::GETGLOBAL, terminalExpr.registerOffset, constIndex);
			}
			if (terminalExpr.flag & AST::FLAG_STORE) {
				terminalExpr.lvalue1 = constIndex;
			}
		} else if (terminalExpr.flag & AST::FLAG_UPVALUE) {
			if (!(terminalExpr.flag & AST::FLAG_NOLOAD)) {
				terminalExpr.registerOffset = register_.allocate();
				terminalExpr.flag |= AST::FLAG_TEMP;
				appendCode_(Instruction::GETUPVAL, terminalExpr.registerOffset,
				            corresponding.registerOffset, corresponding.functionLevel);
			} 
			if (terminalExpr.flag & AST::FLAG_STORE) {
				terminalExpr.lvalue1 = corresponding.functionLevel;
				terminalExpr.lvalue2 = corresponding.registerOffset;
			}
		} else {
			if (terminalExpr.flag & AST::FLAG_LOAD) {
				terminalExpr.registerOffset = register_.allocate();
				terminalExpr.flag |= AST::FLAG_TEMP;
				appendCode_(Instruction::ASSIGN, terminalExpr.registerOffset, corresponding.registerOffset);				
			} else {				
				terminalExpr.registerOffset = corresponding.registerOffset;
			}
		}
	} else {
		uint32_t constIndex = UINT32_MAX; // To remove compiler warning

		switch (terminalExpr.type) {
		case AST::TerminalExpr::INTEGER:
		{
			int32_t integer = wtoi(terminalExpr.lexeme.c_str());
			constIndex = addConstant_(Variable(integer));
			break;
		}
		case AST::TerminalExpr::HEX:
		{
			int32_t integer = whtoi(terminalExpr.lexeme.c_str());
			constIndex = addConstant_(Variable(integer));
			break;
		}
		case AST::TerminalExpr::FLOAT:
		{
			float floating = wtof(terminalExpr.lexeme.c_str());
			constIndex = addConstant_(Variable(floating));
			break;
		}
		case AST::TerminalExpr::NULLTYPE:
		{
			constIndex = addConstant_(Variable(TypeNull));
			break;
		}
		case AST::TerminalExpr::STRING:
		{
			String* constString = new String(terminalExpr.lexeme, &objectManager_);
			constIndex = addConstant_(Variable(TypeString, constString));
			break;
		}
		default: assert(false);
		}

		terminalExpr.registerOffset = register_.allocate();
		appendCode_(Instruction::GETCONST, terminalExpr.registerOffset, constIndex);
	}
}


void CodeGenerator::visit(AST::CallExpr& callExpr)
{
	AST::CallExpr::ArgumentVector &argList = callExpr.argumentList;
	AST::Expression& function = *callExpr.function;

	safeVisit_(callExpr.function.get());
	assert(function.registerOffset != UINT32_MAX);

	callExpr.registerOffset = function.registerOffset;
	uint32_t numArgs = argList.size();
	uint32_t numRets = 1;
	
	std::for_each(argList.begin(), argList.end(),
		[this](decltype(*argList.begin()) i) {
			safeVisit_(i.get());
			assert(i->registerOffset != UINT32_MAX);
		}
	);

	if (argList.size() >= 1) {
		assert(callExpr.registerOffset+1 == argList[0]->registerOffset);
		for (uint32_t i = 0; i < argList.size() - 1; i++) {
			assert(argList[i]->registerOffset+1 == argList[i+1]->registerOffset);
		}
	}

	appendCode_(Instruction::CALL, callExpr.registerOffset, numArgs, numRets);

	std::for_each(argList.rbegin(), argList.rend(),
		[this](decltype(*argList.begin()) i) { register_.deallocate(i->registerOffset); } );

	register_.deallocate(function.registerOffset);

	callExpr.registerOffset = register_.allocate();
	assert(callExpr.registerOffset == function.registerOffset);
}


void CodeGenerator::visit(AST::FunctionExpr& functionExpr)
{
	AST::FunctionDefinition &functionDef = *functionExpr.functionDef;
	safeVisit_(functionExpr.functionDef.get());
	
	assert(functionDef.functionNum != UINT32_MAX); // is it initialized?

	functionExpr.registerOffset = register_.allocate();
	functionExpr.flag |= AST::FLAG_TEMP;
	appendCode_(Instruction::NEWFUNC, functionExpr.registerOffset, functionDef.functionNum);
}


void CodeGenerator::visit(AST::TableExpr& tableExpr)
{
	AST::TableExpr::InitializerVector &initList = tableExpr.initializerList;

	tableExpr.registerOffset = register_.allocate();
	tableExpr.flag |= AST::FLAG_TEMP;

	if (tableExpr.type == AST::TableExpr::ARRAY) {
		appendCode_(Instruction::NEWARRAY, tableExpr.registerOffset);
	} else if (tableExpr.type == AST::TableExpr::TABLE) {
		appendCode_(Instruction::NEWTABLE, tableExpr.registerOffset);
	} else {
		assert(false);
	}

	std::for_each(initList.begin(), initList.end(),
		[this, &tableExpr](decltype(*initList.begin()) i) {
			i->tableOffset = tableExpr.registerOffset;
			safeVisit_(i.get());
		}
	);
}





inline LabelManager::LabelManager()
{
}

inline LabelManager::~LabelManager()
{
}

inline uint32_t LabelManager::newLabel(const uint32_t offset)
{
	labelList_.push_back(offset);
	return labelList_.size() - 1;
}

inline void LabelManager::setOffset(const uint32_t labelNum, const uint32_t offset)
{
	labelList_[labelNum] = offset;
}

inline uint32_t LabelManager::getOffset(const uint32_t labelNum)
{
	return labelList_[labelNum];
}

inline void LabelManager::reset()
{
	labelList_.clear();
}



Register::Register()
: numRegister_(0), maxNumRegister_(0)
{
}

Register::~Register()
{
}


uint32_t Register::allocate()
{
	if (maxNumRegister_ <= numRegister_) {
		maxNumRegister_++;
	}
	return numRegister_++;
}

void Register::deallocate(const AST::Expression& expr)
{
	if (expr.flag & AST::FLAG_LVALUE) {
		if (expr.flag & AST::FLAG_TEMPTABLE) {
			const AST::BinaryExpr& tableExpr = static_cast<const AST::BinaryExpr&>(expr);
			assert(tableExpr.op == AST::BinaryExpr::INDEX);
			deallocate(*tableExpr.second);
			deallocate(*tableExpr.first);
		}
		if (expr.flag & AST::FLAG_TEMP) {
			deallocate(expr.registerOffset);
		}
	} else {
		assert(expr.registerOffset == numRegister_ - 1);
		numRegister_--;
	}
}

void Register::deallocate(uint32_t registerOffset)
{
	assert(registerOffset == numRegister_ - 1);
	numRegister_--;
}

void Register::reset()
{
	maxNumRegister_ = 0;
	numRegister_ = 0;
}

uint32_t Register::maxSize()
{
	return maxNumRegister_;
}



} // namespace "cmm"
