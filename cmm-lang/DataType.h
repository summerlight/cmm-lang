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

                         String(const String&) = delete;
    const String&        operator=(const String&) = delete;

	const std::wstring&  value() const;
	const uint32_t       hashCode() const;

	friend const bool    operator==(const String& lhs, const String& rhs);
	friend const bool    operator< (const String& lhs, const String& rhs);
	friend const bool    operator<=(const String& lhs, const String& rhs);

private:
	virtual              ~String() override;
	virtual void         forEachObject_(const std::function<void(const Object&)>& func) override;

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
	                 Array(const Array&) = delete;
	const Array&     operator=(const Array&) = delete;
	
	Variable         getValue(int32_t key) const;
	bool             setValue(int32_t key, const Variable& value);
	uint32_t         size();

private:
    virtual          ~Array() override;
	virtual void     forEachObject_(const std::function<void(const Object&)>& func) override;
		
	typedef std::vector<Variable> VarArray_;

	VarArray_        array_;
};




class Table : public Object
{
public:
	explicit         Table(ObjectManager* manager);
	                 Table(const Table&) = delete;
	const Table&     operator=(const Table&) = delete;
	
	Variable         getValue(const Variable& key) const;
	void             setValue(const Variable& key, const Variable& value);
	uint32_t         size();

private:
	virtual          ~Table() override;
	virtual void     forEachObject_(const std::function<void(const Object&)>& func) override;

	typedef std::unordered_map<Variable, Variable, Variable::Hash, Variable::StrictEqual>  VarTable_;	

	VarTable_        table_;
};


class Prototype;

class Closure : public Object
{
public:
	explicit               Closure(Ref<Prototype> prototype, Ref<Closure> upperClosure, ObjectManager* objectManager = nullptr);    	
	                       Closure(const Closure&) = delete;
	const Closure&         operator=(const Closure&) = delete;

    Ref<Closure>           upperClosure();

	Variable&              upValue(uint32_t functionLevel, uint32_t offset);

	Variable&              local(uint32_t offset);
	const Variable&        local(uint32_t offset) const;

private:
    virtual                ~Closure() override;
	virtual void           forEachObject_(const std::function<void(const Object&)>& func) override;

	const Ref<Prototype>   prototype_;
	Ref<Closure>           upperClosure_;

	std::vector<Variable>  local_;
};

inline Ref<Closure> Closure::upperClosure()
{
	return upperClosure_.get();
}

inline Variable& Closure::local(uint32_t offset)
{
	assert(offset >= 0 && offset < local_.size());
	return local_[offset];
}

inline const Variable& Closure::local(uint32_t offset) const
{
	assert(offset >= 0 && offset < local_.size());
	return local_[offset];
}


class Function : public Object
{
public:
	explicit              Function(Ref<Prototype> prototype, Ref<Closure> upper, ObjectManager* manager = nullptr);
                          Function(const Function&) = delete;
	const Function&       operator=(const Function&) = delete;

	const Ref<Prototype>  prototype() const;

	Ref<Closure>          upperClosure();

private:
	virtual               ~Function() override;
	virtual void          forEachObject_(const std::function<void(const Object&)>& func) override;

	const Ref<Prototype>  prototype_;
	Ref<Closure>          upperClosure_;
};

inline Ref<Closure> Function::upperClosure()
{
	return upperClosure_;
}



} // namespace "cmm"

#endif