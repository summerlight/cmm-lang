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
	explicit        Context();
	                ~Context();

	void            load(const wchar_t code[]);
	void            run(uint32_t numArgs, uint32_t numRets);
	
	void            registerCfunction(const wchar_t name[], CFunction func);

	void            garbageCollect();

	uint32_t        stackSize();
                  
	Type            type(uint32_t index) const;

	void            pop(uint32_t number = 1);
	void            clear();

	void            pushNull();

	void            pushInt(int32_t value);
	uint32_t        getInt(uint32_t index) const;

	void            pushFloat(float value);
	float           getFloat(uint32_t index) const;

	void            pushString(const wchar_t value[]);
	const wchar_t*  getString(uint32_t index) const;

	void            pushNewTable();
	void            pushTableValue(uint32_t tablePos);
	void            setTableValue(uint32_t tablePos);
	uint32_t        tableSize(uint32_t tablePos) const;
                   
	void            pushNewArray();
	void            pushArrayValue(uint32_t arrayPos, uint32_t arrayIndex);
	void            setArrayValue(uint32_t arrayPos, uint32_t arrayIndex);
	uint32_t        arraySize(uint32_t arrayPos) const;
                   
	void            setGlobal(uint32_t index, const wchar_t globalName[]);
	void            getGlobal(const wchar_t globalName[]);
                   
private:           
	void            loop_();
                   
	void            functionCall_(Variable argValues[], uint32_t numArgs, uint32_t numRets);
	void            CfunctionCall_(Variable argValues[], uint32_t numArgs, uint32_t numRets);
	void            functionReturn_(Variable retValues[], uint32_t numRets);
                   
	void            checkStack_(uint32_t index, Type type, const wchar_t typeName[]) const;
	void            checkStackRange_(uint32_t index) const;
	void            checkStackOverflow_() const;

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