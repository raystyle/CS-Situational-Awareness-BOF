#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"
#ifndef bufsize
#define bufsize 8192
#endif

//#pragma GCC diagnostic ignored "-Wint-conversion"
//formatp output = {1}; // this is just done so its we don't go into .bss which isn't handled properly
char * output = (char*)1;
WORD currentoutsize = 1;
HANDLE trash = (HANDLE)1; // Needed for x64 to not give relocation error
//#pragma GCC diagnostic pop

int bofstart();
#ifdef BOF
void internal_printf(const char* format, ...);
#endif
char * Utf16ToUtf8(const wchar_t* input);
void printoutput(BOOL done);
void bofstop();
#ifdef BOF
int bofstart()
{   
    //output.original=NULL;
    //handle any global initilization here
    //BeaconFormatAlloc(&output, bufsize+256);
    output = (char*)MSVCRT$calloc(bufsize, 1);
    currentoutsize = 0;
    return 1;

}

void internal_printf(const char* format, ...){
    int buffersize = 0;
    int transfersize = 0;
    char * curloc = NULL;
    char* intBuffer = NULL;
    char* transferBuffer = (char*)intAlloc(bufsize);
    va_list args;
    va_start(args, format);
    buffersize = MSVCRT$vsnprintf(NULL, 0, format, args); // +1 because vsprintf goes to buffersize-1 , and buffersize won't return with the null
    va_end(args);
    intBuffer = (char*)intAlloc(buffersize);
    /*Print string to memory buffer*/
    va_start(args, format);
    MSVCRT$vsnprintf(intBuffer, buffersize, format, args); // tmpBuffer2 has a null terminated string
    va_end(args);
    if(buffersize + currentoutsize < bufsize) // If this print doesn't overflow our output buffer, just buffer it to the end
    {
        //BeaconFormatPrintf(&output, intBuffer);
        memcpy(output+currentoutsize, intBuffer, buffersize);
        currentoutsize += buffersize;
    }
    else // If this print does overflow our output buffer, lets print what we have and clear any thing else as it is likely this is a large print
    {
        curloc = intBuffer;
        while(buffersize > 0)
        {
            transfersize = bufsize - currentoutsize; // what is the max we could transfer this request
            if(buffersize < transfersize) //if I have less then that, lets just transfer what's left
            {
                transfersize = buffersize;
            }
            memcpy(output+currentoutsize, curloc, transfersize); // copy data into our transfer buffer
            currentoutsize += transfersize;
            //BeaconFormatPrintf(&output, transferBuffer); // copy it to cobalt strikes output buffer
            if(currentoutsize == bufsize)
            {
            printoutput(FALSE); // sets currentoutsize to 0 and prints
            }
            memset(transferBuffer, 0, transfersize); // reset our transfer buffer
            curloc += transfersize; // increment by how much data we just wrote
            buffersize -= transfersize; // subtract how much we just wrote from how much we are writing overall
        }
    }
    intFree(intBuffer);
    intFree(transferBuffer);
}

void printoutput(BOOL done)
{

    int size = 0;
    char * msg = NULL;
    //msg = BeaconFormatToString(&output, &size);
    BeaconOutput(CALLBACK_OUTPUT, output, currentoutsize);
    currentoutsize = 0;
    memset(output, 0, bufsize);
    if(done) {MSVCRT$free(output); output=NULL;}
}
#endif

char* Utf16ToUtf8(const wchar_t* input)
{
    int ret = Kernel32$WideCharToMultiByte(
        CP_UTF8,
        0,
        input,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    char* newString = (char*)intAlloc(sizeof(char) * ret);

    ret = Kernel32$WideCharToMultiByte(
        CP_UTF8,
        0,
        input,
        -1,
        newString,
        sizeof(char) * ret,
        NULL,
        NULL
    );

    if (0 == ret)
    {
        goto fail;
    }

retloc:
    return newString;
/*location to free everything centrally*/
fail:
    if (newString){
        intFree(newString);
        newString = NULL;
    };
    goto retloc;
}

//release any global functions here
void bofstop()
{

    return;
}
