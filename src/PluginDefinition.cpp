//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <string>
#include "zlib\\zlib.h"

#pragma comment(lib, "ZLib.lib")
using namespace std;

static string ReadFromSkintilla(HWND& hwndSkillaOut);
//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("decode"), Zlib_Decode, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void Zlib_Decode()
{
	HWND hwndSkillaOut = NULL;
	string content = ReadFromSkintilla(hwndSkillaOut);
	inflateInit2(NULL, 47);

	content = "abcdefg";
	::SendMessage(hwndSkillaOut, SCI_SETTEXT, content.size(), (LPARAM)content.c_str());
}

/*
int unzip(char* source, int len, char*des)
{
	int ret, have;
	int offset = 0;
	z_stream d_stream;
	Byte compr[segment_size] = { 0 }, uncompr[segment_size * 4] = { 0 };
	memcpy(compr, (Byte*)source, len);
	uLong comprLen, uncomprLen;
	comprLen = len;//一开始写成了comprlen=sizeof(compr)以及comprlen=strlen(compr)，后来发现都不对。

				   //sizeof(compr)永远都是segment_size，显然不对，strlen(compr)也是不对的，因为strlen只算到\0之前，

				   //但是gzip或者zlib数据里\0很多。
	uncomprLen = segment_size * 4;
	strcpy((char*)uncompr, "garbage");

	d_stream.zalloc = Z_NULL;
	d_stream.zfree = Z_NULL;
	d_stream.opaque = Z_NULL;

	d_stream.next_in = Z_NULL;//inflateInit和inflateInit2都必须初始化next_in和avail_in
	d_stream.avail_in = 0;//deflateInit和deflateInit2则不用

	ret = inflateInit2(&d_stream, 47);
	if (ret != Z_OK)
	{
		printf("inflateInit2error:%d", ret);
		returnret;
	}
	d_stream.next_in = compr;
	d_stream.avail_in = comprLen;
	do
	{
		d_stream.next_out = uncompr;
		d_stream.avail_out = uncomprLen;
		ret = inflate(&d_stream, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);
		switch (ret)
		{
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&d_stream);
			return ret;
		}
		have = uncomprLen - d_stream.avail_out;
		memcpy(des + offset, uncompr, have);//这里一开始我写成了memcpy(des+offset,d_stream.next_out,have);

											//后来发现这是不对的，因为next_out指向的下次的输出，现在指向的是无有意义数据的内存。见下图

		offset += have;

	} while (d_stream.avail_out == 0);
	inflateEnd(&d_stream);
	memcpy(des + offset, "\0", 1);
	return ret;
}
*/
static string ReadFromSkintilla(HWND& hwndSkillaOut)
{
	string result;
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which != -1)
	{
		HWND hCurrentEditView = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

		int cbText = ::SendMessage(hCurrentEditView, SCI_GETLENGTH, NULL, NULL);

		result.resize(cbText + 1);
		::SendMessage(hCurrentEditView, SCI_GETTEXT, result.size(), (LPARAM)result.c_str());

		result.resize(cbText);

		hwndSkillaOut = hCurrentEditView;
	}
	

	return result;
}