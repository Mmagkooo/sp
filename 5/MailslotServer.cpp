#include <windows.h>
#include <stdio.h>
#include <conio.h>

int main()
{
  BOOL   fReturnCode; 
  DWORD  cbMessages;
  DWORD  cbMsgNumber;
  HANDLE hMailslot;
  LPCTSTR  lpszMailslotName = TEXT("\\\\.\\mailslot\\$Channel$");
  char   szBuf[512];
  DWORD  cbRead;

  hMailslot = CreateMailslot(
    lpszMailslotName, 0,
    MAILSLOT_WAIT_FOREVER, NULL);

  if(hMailslot == INVALID_HANDLE_VALUE)
  {
    fprintf(stdout,"CreateMailslot: Error %ld\n", 
      GetLastError());
    _getch();
    return 0;
  }

  fprintf(stdout,"Mailslot created\n"); 
  
  while(1)
  {
    fReturnCode = GetMailslotInfo(
      hMailslot, NULL, &cbMessages,
      &cbMsgNumber, NULL);

    if(!fReturnCode)
    {
      fprintf(stdout,"GetMailslotInfo: Error %ld\n", 
        GetLastError());
      _getch();
      break;
    }

    if(cbMsgNumber != 0)
    {
      if(ReadFile(hMailslot, szBuf, 512, &cbRead, NULL))
      {
        printf("Received: <%s>\n", szBuf);
        if(!strcmp(szBuf, "exit"))
          break;
      }
      else
      {
        fprintf(stdout,"ReadFile: Error %ld\n", 
          GetLastError());
        _getch();
        break;
      }
    }
    Sleep(500);
  }

  CloseHandle(hMailslot);
  return 0;
}