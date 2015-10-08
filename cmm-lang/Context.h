#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <cstdint>
#include <string>
#include <vector>

#include "Memory.h"
#include "Object.h"
#include "DataType.h"

namespace cmm
{

class Context
{
public:
	explicit          Context();
	                  ~Context();

	void              load(const wchar_t code[]);
	void              run(const uint32_t numArgs, const uint32_t numRets);
	
	void              registerCfunction(const wchar_t name[], CFunction func);

	void              garbageCollect();

	const uint32_t    stackSize();

	const Type        type(const uint32_t index) const;

	void              pop(const uint32_t number = 1);
	void              clear();

	void              pushNull();

	void              pushInt(const int32_t value);
	const uint32_t    getInt(const uint32_t index) const;

	void              pushFloat(const float value);
	const float       getFloat(const uint32_t index) const;

	void              pushString(const wchar_t value[]);
	const wchar_t*    getString(const uint32_t index) const;

	void              pushNewTable();
	void              pushTableValue(const uint32_t tablePos);
	void              setTableValue(const uint32_t tablePos);
	const uint32_t    tableSize(const uint32_t tablePos) const;

	void              pushNewArray();
	void              pushArrayValue(const uint32_t arrayPos, const uint32_t arrayIndex);
	void              setArrayValue(const uint32_t arrayPos, const uint32_t arrayIndex);
	const uint32_t    arraySize(const uint32_t arrayPos) const;

	void              setGlobal(const uint32_t index, const wchar_t globalName[]);
	void              getGlobal(const wchar_t globalName[]);

private:
	void              loop_();

	void              functionCall_(Variable argValues[], const uint32_t numArgs, const uint32_t numRets);
	void              CfunctionCall_(Variable argValues[], const uint32_t numArgs, const uint32_t numRets);
	void              functionReturn_(Variable retValues[], const uint32_t numRets);

	void              checkStack_(const uint32_t index, const Type type, const wchar_t typeName[]) const;
	void              checkStackRange_(const uint32_t index) const;
	void              checkStackOverflow_() const;

	struct CallInfo_
	{
		CallInfo_(Ref<Function> func, Ref<Closure> cl, Variable* retTo, uint32_t numRets, uint32_t pc)
		: function(func), closure(cl), returnTo(retTo), numRets(numRets), programCounter(pc) {}

		Ref<Function>  function;
		Ref<Closure>   closure;
		Variable*      returnTo;
		uint32_t       numRets;
		uint32_t       programCounter;
	};

	typedef std::vector<CallInfo_> CallStack_;
	typedef std::vector<Variable> VariableVector_;
	
	ObjectManager     objectManager_;
	Ref<Table>        global_;
	VariableVector_   buffer_;
	uint32_t          bufferSize_;
	CallStack_        callStack_;

	bool              reentrant_;	
};

} // The end of the namespace "cmm"

#endif