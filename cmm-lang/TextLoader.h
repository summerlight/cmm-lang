#ifndef TEXT_LOADER_H
#define TEXT_LOADER_H

#include <windows.h>
#include <fstream>

using std::ifstream;
using std::ios_base;

class TextLoader
{
	enum Mode {UTF8, UTF16LE, UTF16BE, ANSI};

public:
	explicit          TextLoader();
	                  ~TextLoader();

	const bool        load(const wchar_t FileName[]);
	const wchar_t*    string();
	void              clear();

private:
	                  TextLoader(const TextLoader&);
	const TextLoader& operator=(const TextLoader&);

	const bool        loadFileToBuffer_(const wchar_t FileName[]);
	void              checkFileMode_();
	void              bigEndianToLittleEndian_();
	void              convertToUTF16_();

	char              *buffer_;
	wchar_t           *wbuffer_;
	long              size_;
	long              wsize_;
	Mode              mode_;	
};

inline const wchar_t* TextLoader::string()
{
	return wbuffer_;
}

#endif