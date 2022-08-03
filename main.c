#include <stdio.h>
#include <Windows.h>

LPSTR shell(const wchar_t* cmd)
{
    LPSTR result = (char*)malloc(24000 * sizeof(char));
    ZeroMemory(result, strlen(result));
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;


    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
        return result;

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return result;
    }

    BOOL bProcessEnded = FALSE;
    for (; !bProcessEnded;)
    {
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        for (;;)
        {
            char buf[24000];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail)
                break;

            if (!ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                break;

            buf[dwRead] = 0;
            strcat_s(result, 24000, buf);
        }
    }

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return result;
}

int main()
{
    wchar_t* command = (wchar_t*)malloc(24000 * sizeof(wchar_t));
    ZeroMemory(command, lstrlen(command));
    wcscpy_s(command, 255, L"cmd.exe /c hostname");
    LPSTR name = shell(command);

    printf("%s\n", name);

    free(command);
    
    return 0;
}
