#ifndef COMPILER_H
#define COMPILER_H

#include <cstdint>
#include <string>

#include "Object.h"

namespace cmm {

class Prototype;
class Error;
class ObjectManager;

class Compiler
{
public:
	explicit          Compiler(ObjectManager& objectManager);
	                  ~Compiler();

	std::wstring&     ASTString();
	std::wstring&     byteCode();

	Ref<Prototype>    compile(const wchar_t code[], const bool drawTree = false, const bool printCode = false);

private:
	                  Compiler(const Compiler&);
	const Compiler&   operator=(const Compiler&);

	ObjectManager     &objectManager_;

	std::wstring      treeString_;
	std::wstring      codeString_;
};

} // The end of namespace "cmm"



#endif