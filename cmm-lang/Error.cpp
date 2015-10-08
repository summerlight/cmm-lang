#include "StdAfx.h"
#include "Error.h"

#include <cstdarg>
#include <iostream>

namespace cmm
{

Error::Error(const wchar_t errorMsg[], ...)
{
	va_list args;
	va_start( args, errorMsg );
	wchar_t buffer[256];

	vswprintf(buffer, 256, errorMsg, args);

	errorMsg_ = std::wstring(buffer);
}

Error::~Error()
{
}

const std::wstring& Error::errorStr() const
{
	return errorMsg_;
}

} // namespace "cmm"