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
#include "zlib\\zlib.h"
#include <vector>

using namespace std;
bool ZlibDecode(vector<uint8_t>& needReplaceText, vector<uint8_t>& decoded);
bool ReadNeedReplaceText(vector<uint8_t>& needReplaceText, HWND& curScintilla);

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
    setCommand(0, TEXT("decode"), ZlibDecode, NULL, false);
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
void ZlibDecode()
{
	vector<uint8_t> needReplaceText;
	HWND curScintilla = NULL;
	if (ReadNeedReplaceText(needReplaceText, curScintilla) == false)
	{
		return;
	}

	vector<uint8_t> decoded;
	if (ZlibDecode(needReplaceText, decoded) == false)
	{
		return;
	}
	//::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)decoded.c_str());
}




bool GetCurrentScintilla(HWND& curScintilla)
{
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
	{
		return false;
	}
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	return true;
}


bool ReadNeedReplaceText(vector<uint8_t>& needReplaceText, HWND& curScintilla)
{
	if (GetCurrentScintilla(curScintilla) == false)
	{
		return false;
	}

	size_t start = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
	size_t end = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
	if (end < start)
	{
		size_t tmp = start;
		start = end;
		end = tmp;
	}

	size_t asciiTextLen = end - start;
	if (asciiTextLen == 0)
	{
		size_t allTextlength = ::SendMessage(curScintilla, SCI_GETLENGTH, 0, 0);
		::SendMessage(curScintilla, SCI_SETSELECTIONSTART, 0, 0);
		::SendMessage(curScintilla, SCI_SETSELECTIONEND, allTextlength, 0);
		needReplaceText.resize(allTextlength);
	}
	else 
	{
		needReplaceText.resize(asciiTextLen);
	}

	::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)(&needReplaceText[0]));
	return true;
}


bool ZlibDecode(vector<uint8_t>& needReplaceText, vector<uint8_t>& decoded)
{ 
	z_stream strm;
	memset(&strm, 0, sizeof(z_stream));
	size_t full_length = needReplaceText.size();;
	size_t half_length = full_length / 2;
	
	decoded.resize(full_length + half_length);
	bool done = false;
	int status = 0;

	
	strm.next_in = (Bytef *)(&needReplaceText[0]);
	strm.avail_in = (unsigned int)needReplaceText.size();
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;

	if (inflateInit2(&strm, (15 + 32)) != Z_OK)
	{
		return false;
	}

	while (done == false)
	{
		// Make sure we have enough room and reset the lengths.
		if (strm.total_out >= decoded.size())
		{
			decoded.resize(decoded.size() + half_length);
		}
		strm.next_out = (Bytef *)(&decoded[0]) + strm.total_out;
		strm.avail_out = (unsigned int)(decoded.size() - strm.total_out);

		
		status = inflate(&strm, Z_SYNC_FLUSH);
		if (status == Z_STREAM_END)
		{
			done = true;
		}
		else if (status != Z_OK)
		{
			break;
		}
	}
	if (inflateEnd(&strm) != Z_OK)
	{
		return false;
	}

	// Set real length.
	if (done)
	{
		decoded.resize(strm.total_out);
	}
	return done;
}