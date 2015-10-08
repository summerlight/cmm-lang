#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "Object.h"
#include "Memory.h"

namespace cmm
{

class String : public Object
{
public:
	explicit             String(ObjectManager* manager);
	                     String(const wchar_t value[], ObjectManager* manager = nullptr);
	                     String(const std::wstring& value, ObjectManager* manager = nullptr);

	const std::wstring&  value() const;
	const uint32_t       hashCode() const;

	friend const bool    operator==(const String& lhs, const String& rhs);
	friend const bool    operator< (const String& lhs, const String& rhs);
	friend const bool    operator<=(const String& lhs, const String& rhs);

private:
	                     String(const String&);
	const String&        operator=(const String&);

	virtual              ~String();
	virtual void         forEachObject_(const std::function<void(const Object&)>& func);

	const uint32_t       calculateHashCode();

	std::wstring         value_;
	uint32_t             hashCode_;	
};

inline const bool operator==(const String& lhs, const String& rhs)
{
	return lhs.value_ == rhs.value_;
}

inline const bool operator< (const String& lhs, const String& rhs)
{
	return lhs.value_ < rhs.value_;
}

inline const bool operator<=(const String& lhs, const String& rhs)
{
	return lhs.value_ <= rhs.value_;
}


class Array : public Object
{
public:
	explicit         Array(ObjectManager* manager);
	
	const Variable   getValue(const int32_t key) const;
	const bool       setValue(const int32_t key, const Variable& value);
	const uint32_t   size();

private:
	                 Array(const Array&);
	const Array&     operator=(const Array&);

	virtual          ~Array();
	virtual void     forEachObject_(const std::function<void(const Object&)>& func);
		
	typedef std::vector<Variable> VarArray_;

	VarArray_        array_;
};




class Table : public Object
{
public:
	explicit         Table(ObjectManager* manager);
	
	const Variable   getValue(const Variable& key) const;
	void             setValue(const Variable& key, const Variable& value);
	const uint32_t   size();

private:
	                 Table(const Table&);
	const Table&     operator=(const Table&);

	virtual          ~Table();
	virtual void     forEachObject_(const std::function<void(const Object&)>& func);

	typedef std::unordered_map<Variable, Variable, Variable::Hash, Variable::StrictEqual>  VarTable_;	

	VarTable_        table_;
};


class Prototype;

class Closure : public Object
{
public:
	explicit               Closure(const Ref<Prototype> prototype, Ref<Closure> upperClosure, ObjectManager* objectManager = nullptr);
	                       ~Closure();

	Ref<Closure>           upperClosure();
	const Ref<Closure>     upperClosure() const;

	Variable&              upValue(const uint32_t functionLevel, const uint32_t offset);

	Variable&              local(const uint32_t offset);
	const Variable&        local(const uint32_t offset) const;

private:
	                       Closure(const Closure&);
	const Closure&         operator=(const Closure&);

	virtual void           forEachObject_(const std::function<void(const Object&)>& func);

	const Ref<Prototype>   prototype_;
	Ref<Closure>           upperClosure_;

	std::vector<Variable>  local_;
};

inline Ref<Closure> Closure::upperClosure()
{
	return upperClosure_.get();
}

inline const Ref<Closure> Closure::upperClosure() const
{
	return upperClosure_.get();
}

inline Variable& Closure::local(const uint32_t offset)
{
	assert(offset >= 0 && offset < local_.size());
	return local_[offset];
}

inline const Variable& Closure::local(const uint32_t offset) const
{
	assert(offset >= 0 && offset < local_.size());
	return local_[offset];
}


class Function : public Object
{
public:
	explicit              Function(const Ref<Prototype> prototype, Ref<Closure> upper, ObjectManager* manager = nullptr);

	const Ref<Prototype>  prototype() const;

	Ref<Closure>          upperClosure();
	const Ref<Closure>    upperClosure() const;

private:
	                      Function(const Function&);
	const Function&       operator=(const Function&);

	virtual               ~Function();
	virtual void          forEachObject_(const std::function<void(const Object&)>& func);

	const Ref<Prototype>  prototype_;
	Ref<Closure>          upperClosure_;
};

inline Ref<Closure> Function::upperClosure()
{
	return upperClosure_;
}

inline const Ref<Closure> Function::upperClosure() const
{
	return upperClosure_;
}



} // The end of namespace "cmm"

#endif