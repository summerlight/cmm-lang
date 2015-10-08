#ifndef POSITION_H
#define POSITION_H

#include <cstdint>
#include <cassert>

namespace cmm
{

struct Position
{
public:
	Position();
	Position(const uint32_t fromLine, const uint32_t toLine, const uint32_t fromCol, const uint32_t toCol);
	Position(const Position& from, const Position& to);

	// Generally, below member variables should be private member.
	// However this class is more of data structure representation of the token's position
	// rather than a single object with certain behavior, so I made them as public members.
	
	uint32_t	startLine;
	uint32_t	startCol;
	uint32_t	endLine;
	uint32_t	endCol;
};

inline Position::Position() : startLine(0), endLine(0), startCol(0), endCol(0)
{
	// Intentionally empty function
}

inline Position::Position(const Position& from, const Position& to)
	: startLine(from.startLine), endLine(to.endLine), startCol(from.startCol), endCol(from.endCol)
{
	// Intentionally empty function
}

inline Position::Position(const uint32_t fromLine, const uint32_t toLine, const uint32_t fromCol, const uint32_t toCol)
	: startLine(fromLine), endLine(toLine), startCol(fromCol), endCol(toCol)
{
	assert (fromLine >= 0 && toLine >= 0);
	assert (fromCol >= 0 && toCol >= 0);
}

}

#endif