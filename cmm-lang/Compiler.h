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
                      Compiler(const Compiler&) = delete;
    const Compiler&   operator=(const Compiler&) = delete;

	std::wstring&     ASTString();
	std::wstring&     byteCode();

	Ref<Prototype>    compile(const wchar_t code[], bool drawTree = false, bool printCode = false);

private:
	ObjectManager     &objectManager_;

	std::wstring      treeString_;
	std::wstring      codeString_;
};

} // namespace "cmm"



#endif