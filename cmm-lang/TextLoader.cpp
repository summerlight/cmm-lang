#include "StdAfx.h"
#include "TextLoader.h"

#include <cassert>

#include <memory>

TextLoader::TextLoader()
: buffer_(nullptr), wbuffer_(nullptr)
{
}

const bool TextLoader::load(const wchar_t FileName[])
{
	clear();

	if (!loadFileToBuffer_(FileName)) {
		return false;
	}
		
	checkFileMode_();
	convertToUTF16_();

	return true;
}

const bool TextLoader::loadFileToBuffer_(const wchar_t FileName[])
{
	ifstream file(FileName, ios_base::binary | ios_base::in);

	if(!file) {
		return false;
	}
	
	file.seekg(0, ios_base::end);

	std::streamoff fileSize = file.tellg();

	if (fileSize > INT32_MAX) {
		file.close();
		return false;
	} else {
		size_ = static_cast<long>(fileSize);
	}
	buffer_ = new char[size_];	
	file.seekg(0);
	file.read(buffer_, size_);
	file.close();

	return true;
}

void TextLoader::checkFileMode_()
{
	int *header = reinterpret_cast<int*>(buffer_);

	if (*header << 16 == 0xFEFF0000)
		mode_ = UTF16LE;
	else if (*header << 16 == 0xFFFE0000)
		mode_ = UTF16BE;
	else if (*header << 8 == 0xBFBBEF00)
		mode_ = UTF8;
	else
		mode_ = ANSI;
}

void TextLoader::bigEndianToLittleEndian_()
{
	assert(size_ % 2 == 0);

	for(int i = 0; i < size_; i += 2) {
		char temp;

		temp = buffer_[i];
		buffer_[i] = buffer_[i+1];
		buffer_[i+1] = temp;
	}
}

void TextLoader::convertToUTF16_()
{
	switch(mode_) {
	case ANSI:
		wsize_ = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer_, size_, wbuffer_, 0);
		wbuffer_ = new wchar_t[wsize_+1];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer_, size_, wbuffer_, wsize_);
		wbuffer_[wsize_] = 0;
		delete[] buffer_;
		buffer_ = nullptr;
		break;

	case UTF8:
		wsize_ = MultiByteToWideChar(CP_UTF8, 0, buffer_+3, size_-3, wbuffer_, 0);
		wbuffer_ = new wchar_t[wsize_+1];
		MultiByteToWideChar(CP_UTF8, 0, buffer_+3, size_-3, wbuffer_, wsize_);
		wbuffer_[wsize_] = 0;
		delete[] buffer_;
		buffer_ = nullptr;
		break;

	case UTF16LE:
		wsize_ = size_/2 - 1;
		wbuffer_ = reinterpret_cast<wchar_t*>(buffer_+2);
		break;

	case UTF16BE:
		bigEndianToLittleEndian_();
		wsize_ = size_/2 - 1;
		wbuffer_ = reinterpret_cast<wchar_t*>(buffer_+2);
		break;

	default:
		break;
	};
}

void TextLoader::clear()
{
	delete[] buffer_;

	if (mode_ == ANSI || mode_ == UTF8) {
		delete[] wbuffer_;
	}

	buffer_ = nullptr;
	wbuffer_ = nullptr;
}

TextLoader::~TextLoader()
{
	clear();
}
