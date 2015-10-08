#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace cmm
{

class Error
{
public:
	explicit              Error();
	explicit              Error(const wchar_t errorMsg[], ...);
	virtual               ~Error();

	const std::wstring&   errorStr() const;

private:
	std::wstring	      errorMsg_;

};

} // namespace "cmm"

#endif