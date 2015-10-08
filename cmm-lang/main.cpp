#include "StdAfx.h"

#include <cstdio>
#include <locale>
#include <iostream>

#include "cmm.h"
#include "TextLoader.h"

void print(cmm::Context& context)
{
	if (context.stackSize() != 0) {
		switch(context.type(0)) {
		case cmm::TypeNull:   wprintf(L"null\n"); break;
		case cmm::TypeInt:    wprintf(L"%d\n", context.getInt(0)); break;
		case cmm::TypeFloat:  wprintf(L"%f\n", context.getFloat(0)); break;
		case cmm::TypeString: wprintf(L"%s\n", context.getString(0)); break;
		case cmm::TypeArray:  wprintf(L"array\n"); break;
		case cmm::TypeTable:  wprintf(L"table\n"); break;
		case cmm::TypeFunc:   wprintf(L"function\n"); break;
		case cmm::TypeCFunc:  wprintf(L"C function\n"); break;
		}
	}
	context.clear();
}

void size(cmm::Context& context)
{
	int32_t size = -1;

	if (context.stackSize() != 0) {		
		switch(context.type(0)) {
		case cmm::TypeString: size = wcslen(context.getString(0)); break;
		case cmm::TypeArray:  size = context.arraySize(0); break;
		case cmm::TypeTable:  size = context.tableSize(0); break;
		default:              break;
		}	
	}

	context.clear();
	if (size == -1) {
		context.pushNull();
	} else {
		context.pushInt(size);
	}
}

void RunInterpreter(cmm::Context& context)
{
	std::wcout << L"C-- 0.01 by Summerlight" << std::endl;
	std::wcout << L"* Press enter once more to execute buffered code." << std::endl << std::endl;

	for (;;) {
		std::wstring buffer;
		uint32_t lineNum = 0;
		
		for(;;) {
			std::wstring temp;
			std::wcout << lineNum++ << L" > ";
			std::getline(std::wcin, temp);

			if (temp == L"") { lineNum = 0; break; }

			buffer.append(std::move(temp));
		}
		 
		try {
			context.load(buffer.c_str());
			context.run(0, 0);
		} catch (cmm::Error& error) {
			std::wcout << error.errorStr() << std::endl;
		}

		context.garbageCollect();
	}
}

void RunFile(cmm::Context& context, std::wstring& fileName)
{
	TextLoader loader;

	if (loader.load(fileName.c_str()) == false) {
		std::wcout << L"File " << fileName << L" does not exist." << std::endl;
		return;
	}

	try {
		context.load(loader.string());
		context.run(0, 0);
		
		context.getGlobal(L"main");
		context.run(0, 0);
	} catch (cmm::Error& error) {
		std::wcout << error.errorStr() << std::endl;
	}
}

int wmain(int argc, wchar_t* argv[])
{
	std::locale::global(std::locale("kor"));

	std::wcin.imbue(std::locale("korean"));
	std::wcout.imbue(std::locale("korean")); 

	cmm::Context context;
	context.registerCfunction(L"print", print);
	context.registerCfunction(L"sizeof", size);

	if (argc < 2) {
		RunInterpreter(context);
	} else {
		std::wstring fileName(argv[1]);

		RunFile(context, fileName);
	}
}