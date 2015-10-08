#include <memory>

namespace cmm
{

namespace AST
{
struct Base;
struct Statement;
struct Expression;
struct StmtSequence;
struct TableInitializer;
struct FunctionDefinition;
struct CompoundStmt;
struct LoopStmt;
struct ForStmt;
struct WhileStmt;
struct DoWhileStmt;
struct IfElseStmt;
struct ReturnStmt;
struct JumpStmt;
struct VariableStmt;
struct ExpressionStmt;
struct UnaryExpr;
struct BinaryExpr;
struct TrinaryExpr;
struct TerminalExpr;
struct CallExpr;
struct FunctionExpr;
struct TableExpr;
}

typedef std::unique_ptr<AST::Base> BasePtr;
typedef std::unique_ptr<AST::Statement> StatementPtr;
typedef std::unique_ptr<AST::Expression> ExpressionPtr;
typedef std::unique_ptr<AST::StmtSequence> StmtSequencePtr;
typedef std::unique_ptr<AST::TableInitializer> TableInitPtr;
typedef std::unique_ptr<AST::FunctionDefinition> FunctionDefPtr;
typedef std::unique_ptr<AST::CompoundStmt> CompoundStmtPtr;
typedef std::unique_ptr<AST::LoopStmt> LoopStmtPtr;
typedef std::unique_ptr<AST::ForStmt> ForStmtPtr;
typedef std::unique_ptr<AST::WhileStmt> WhileStmtPtr;
typedef std::unique_ptr<AST::DoWhileStmt> DoWhileStmtPtr;
typedef std::unique_ptr<AST::IfElseStmt> IfElseStmtPtr;
typedef std::unique_ptr<AST::ReturnStmt> ReturnStmtPtr;
typedef std::unique_ptr<AST::JumpStmt> JumpStmtPtr;
typedef std::unique_ptr<AST::VariableStmt> VariableStmtPtr;
typedef std::unique_ptr<AST::ExpressionStmt> ExpressionStmtPtr;
typedef std::unique_ptr<AST::UnaryExpr> UnaryExprPtr;
typedef std::unique_ptr<AST::BinaryExpr> BinaryExprPtr;
typedef std::unique_ptr<AST::TrinaryExpr> TrinaryExprPtr;
typedef std::unique_ptr<AST::TerminalExpr> TerminalExprPtr;
typedef std::unique_ptr<AST::FunctionExpr> FunctionExprPtr;
typedef std::unique_ptr<AST::CallExpr> CallExprPtr;
typedef std::unique_ptr<AST::TableExpr> TableExprPtr;

}