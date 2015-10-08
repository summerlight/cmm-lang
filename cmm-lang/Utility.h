#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>

namespace cmm {

#define BinaryOpFuncObject(name, oper)\
struct name {\
	template <typename T1, typename T2> \
	static inline auto op(T1 a, T2 b) -> decltype(a oper b)\
	{ return a oper b; }\
}

#define UnaryOpFuncObject(name, oper)\
struct name {\
	template <typename T1> \
	static inline auto op(T1 a) -> decltype(oper a)\
	{ return oper a; }\
}

// Arithmetic Operator Function Objects
BinaryOpFuncObject(OpAdd, +);
BinaryOpFuncObject(OpSubtract, -);
BinaryOpFuncObject(OpMultiply, *);
BinaryOpFuncObject(OpDivide, /);
BinaryOpFuncObject(OpModular, %);
UnaryOpFuncObject (OpNeg, -);


// Bit Operator Function Objects
BinaryOpFuncObject(OpBitAnd, &);
BinaryOpFuncObject(OpBitOr, |);
BinaryOpFuncObject(OpBitXor, ^);
BinaryOpFuncObject(OpShiftLeft, <<);
BinaryOpFuncObject(OpShiftRight, >>);
UnaryOpFuncObject (OpBitNot, ~);

// Logic Operator Function Objects
BinaryOpFuncObject(OpLogicAnd, &&);
BinaryOpFuncObject(OpLogicOr, |!);
BinaryOpFuncObject(OpEqual, ==);
BinaryOpFuncObject(OpNotEqual, !=);
BinaryOpFuncObject(OpLess, <);
BinaryOpFuncObject(OpLessEqual, <=);
UnaryOpFuncObject (OpLogicNot, !);


struct Node
{
	Node()
	:prev(nullptr), next(nullptr)
	{
		listHeadInit();
	}

	void  listHeadInit()
	{
		prev = this;
		next = this;
	}

	void  insertBack(Node* insertedNode)
	{
		// target node will be inserted after this node
		assert(prev->next == next->prev);
		insertedNode->next = next;
		insertedNode->prev = this;

		next->prev = insertedNode;
		next = insertedNode;
		assert(prev->next == next->prev);
	}

	void  insertFront(Node* insertedNode)
	{
		// target node will be inserted before this node
		assert(prev->next == next->prev);
		insertedNode->next = this;
		insertedNode->prev = prev;

		prev->next = insertedNode;
		prev = insertedNode;
		assert(prev->next == next->prev);
	}

	void  pickOut()
	{
		assert(prev->next == next->prev);
		next->prev = prev;
		prev->next = next;

		prev = this;
		next = this;
	}

	Node  *prev;
	Node  *next;
};

} // namespace"cmm"
#endif