#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <cstdint>
#include <vector>

#include "Object.h"
#include "ASTVisitor.h"
#include "Instruction.h"
#include "Prototype.h"

namespace cmm
{

class Prototype;
class Program;
class ConstantTable;
class ByteCode;
class Error;
struct Variable;

class LabelManager
{
public:
	explicit            LabelManager();
	                    ~LabelManager();

	const uint32_t      newLabel(const uint32_t offset = UINT32_MAX);
	void                setOffset(const uint32_t labelNum, const uint32_t offset);
	const uint32_t      getOffset(const uint32_t labelNum);
	void                reset();

private:
	                    LabelManager(const LabelManager&);
	const LabelManager& operator=(const LabelManager&);

	typedef std::vector<uint32_t> LabelVector;

	LabelVector         labelList_;
};




class Register
{
public:
	explicit          Register();
	                  ~Register();

	const uint32_t    allocateVariable();
	const uint32_t    allocateTempRegister();
	const uint32_t    allocate();
	void              deallocate(const uint32_t registerOffset);
	void              deallocate(const AST::Expression& expr);
	void              reset();
	const uint32_t    maxSize();

private:
	                  Register(const Register&);
	Register&         operator=(const Register&);

	uint32_t          numRegister_;
	uint32_t          maxNumRegister_;
};





class CodeGenerator : public AST::Visitor
{
public:
	explicit            CodeGenerator(ObjectManager& objectManager);
	                    ~CodeGenerator();

	Ref<Prototype>      createPrototype(AST::FunctionDefinition& functionDef);

	virtual void        visit(AST::StmtSequence& stmtSequence);
	virtual void        visit(AST::TableInitializer& tableInit);	
	virtual void        visit(AST::FunctionDefinition& functionDef);

	virtual void        visit(AST::CompoundStmt& compoundStmt);
	virtual void        visit(AST::ForStmt& forStmt);
	virtual void        visit(AST::WhileStmt& whileStmt);
	virtual void        visit(AST::DoWhileStmt& doWhileStmt);
	virtual void        visit(AST::IfElseStmt& ifElseStmt);	
	virtual void        visit(AST::ReturnStmt& returnStmt);
	virtual void        visit(AST::JumpStmt& jumpStmt);
	virtual void        visit(AST::VariableStmt& variableStmt);
	virtual void        visit(AST::ExpressionStmt& expressionStmt);

	virtual void        visit(AST::UnaryExpr& unaryExpr);
	virtual void        visit(AST::BinaryExpr& binaryExpr);
	virtual void        visit(AST::TrinaryExpr& trinaryExpr);
	virtual void        visit(AST::TerminalExpr& terminalExpr);
	virtual void        visit(AST::CallExpr& callExpr);
	virtual void        visit(AST::FunctionExpr& functionExpr);
	virtual void        visit(AST::TableExpr& tableExpr);

private:
	                    CodeGenerator(const CodeGenerator&);
	CodeGenerator       operator=(const CodeGenerator&);

	const bool          safeVisit_(AST::Base* host);

	const uint32_t      addConstant_(const Variable& constant);
	void                substitueLabelToOffset();

	void                appendCode_(const Instruction::Opcode op, const int32_t operand1,
	                                const int32_t operand2 = 0, const int32_t operand3 = 0);
	const uint32_t      nextOffset_();

	void                appendUnaryOp_(AST::UnaryExpr& unaryExpr, const Instruction::Opcode op);
	void                appendPrefixOp_(AST::UnaryExpr& unaryExpr, const Instruction::Opcode op);
	void                appendPostfixOp_(AST::UnaryExpr& unaryExpr, const Instruction::Opcode op);

	void                appendBinaryOp_(AST::BinaryExpr& binaryExpr, const Instruction::Opcode op, const bool inverted = false);
	void                appendAssignOp_(AST::BinaryExpr& binaryExpr, const Instruction::Opcode Op);
	void                appendAssignOp_(AST::BinaryExpr& binaryExpr);
	void                appendShortCutLogic_(AST::BinaryExpr& binaryExpr);
	void                appendTableLoadOp_(AST::BinaryExpr& binaryExpr);

	void                appendStoreOp_(const AST::Expression& value, const AST::Expression& dest);
	void                appendStoreOp_(const uint32_t valueRegister, const AST::Expression& dest);

	ObjectManager&      objectManager_;
	Ref<Prototype>      prototype_;
	LabelManager        labelManager_;
	Register            register_;
};

} // The end of namespace "cmm"

#endif