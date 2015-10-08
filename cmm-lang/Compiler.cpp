#include "StdAfx.h"
#include "Compiler.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "AST.h"
#include "ASTDrawer.h"
#include "Scanner.h"
#include "Parser.h"
#include "Analyzer.h"
#include "CodeGenerator.h"
#include "CodePrinter.h"

namespace cmm {

Compiler::Compiler(ObjectManager& objectManager)
: objectManager_(objectManager), treeString_(), codeString_()
{
}

Compiler::~Compiler()
{
}

Ref<Prototype> Compiler::compile(const wchar_t code[], bool drawTree, bool printCode)
{
	Parser parser(code);
	FunctionDefPtr rootFunction = parser.parse();
	
	Analyzer analyzer;
	analyzer.analyze(*rootFunction.get());
		
	if (drawTree == true) {
		ASTDrawer drawer;
		//treeString_ = drawer.draw(*rootFunction.get());
		std::wofstream f;
		f.open(L"tree.txt");
		f << drawer.draw(*rootFunction.get());
		f.close();
	}

	CodeGenerator codeGenerator(objectManager_);
	Ref<Prototype> rootPrototype(codeGenerator.createPrototype(*rootFunction));

	if (printCode == true) {
		CodePrinter printer;
		//codeString_ = printer.print(*rootPrototype.get());
		std::wofstream f;
		f.open(L"code.txt");
		f << printer.print(*rootPrototype.get());
		f.close();
	}

	return rootPrototype;
}

std::wstring& Compiler::ASTString()
{
	return treeString_;
}

std::wstring& Compiler::byteCode()
{
	return codeString_;
}

} // namespace "cmm"