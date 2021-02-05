#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#define BUFSIZE MAX_PATH

struct FOO {
   char* str;
   int   num;
};

__declspec(dllexport) char* __stdcall passPtrFOO(struct FOO* ptrFoo) {

    char buf[200];

    if (! ptrFoo) {
  
        MessageBox(0, "ptrFoo is null", "passPtrFOO", 0);
  
     // return 0 (false) to indicate that ptrFoo was a null pointer
        return 0;
    }
  
  
   
  
	
	
	
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	
	const char* prefix = "C:\\Users\\fabio\\Desktop\\jabcode-master\\transfer\\jabcodeWriter.exe --input \"";
	const char* postfix = "\" --output tmpjabcode.png";
	char* cmd = malloc(strlen(prefix)+strlen(ptrFoo->str)+strlen(postfix));
	strcpy(cmd, prefix); /* copy name into the new var */
	strcat(cmd, ptrFoo->str); /* add the extension */
	strcat(cmd, postfix);
	
	//MessageBox(0, cmd, "cmd", 0);  

	LPTSTR szCmdline = _tcsdup(TEXT(cmd));
	
	
	TCHAR Buffer[BUFSIZE];
	DWORD dwRet;

	SetCurrentDirectory(getenv( "TEMP" ));
   
	dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
	//_tprintf(TEXT("Current directory (%s)\n"), Buffer);
	
	/*
	MessageBox(
        NULL,
        Buffer,
        "Debug",
        MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
    );*/
	
	
	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		szCmdline,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
	) 
	{
		printf( "CreateProcess failed (%d).\n", GetLastError() );
		return "";
	}
	
	const char* name = "\\tmpjabcode.png";
	const char* path = getenv( "TEMP" );

	
	char* filename = malloc(strlen(path)+strlen(name)); 
	strcpy(filename, path); 
	strcat(filename, name); 
	
	//MessageBox(0, ptrFoo->str, "passPtrFOO", 0);
	
  
 // return -1 (true) to indicate that ptrFoo was not a null pointer
    return filename;

}
