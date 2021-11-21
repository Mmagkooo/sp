#include <windows.h>
#include <stdio.h>
#include <conio.h>

DWORD main(int argc, char* argv[])
{
    HANDLE hMailslot;
    char   szBuf[512];
    DWORD  cbWritten;

    hMailslot = CreateFile(
        L"\\\\.\\mailslot\\$Channel$", GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (hMailslot == INVALID_HANDLE_VALUE)
    {
        fprintf(stdout, "CreateFile: Error %ld\n",
            GetLastError());
        _getch();
        return 0;
    }

    fprintf(stdout, "\nConnected. Type 'exit' to terminate\n");

    while (1)
    {
        gets_s(szBuf);
        if (!WriteFile(hMailslot, szBuf, strlen(szBuf) + 1,
            &cbWritten, NULL))
            break;
        if (!strcmp(szBuf, "exit"))
            break;
    }
    CloseHandle(hMailslot);
    return 0;
}