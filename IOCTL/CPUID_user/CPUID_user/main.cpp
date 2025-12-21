#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include "ioctl.h"

extern "C" void GetCpuID(char* buffer);
extern "C" bool DetectVmxSupport();

void PrintAppearance()
{
    printf("========================================\n");
    printf("      CPU ID & VT-x Support Checker     \n");
    printf("========================================\n");
}

bool
TestIoctl(HANDLE Handle)
{
    char  OutputBuffer[1000];
    char  InputBuffer[1000];
    ULONG BytesReturned;
    BOOL  Result;

    //
    // Performing METHOD_BUFFERED
    //
    strcpy_s(InputBuffer, "This String is from User Application; using METHOD_BUFFERED");

    printf("\nCalling DeviceIoControl METHOD_BUFFERED:\n");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));

    Result = DeviceIoControl(Handle,
        (DWORD)IOCTL_SIOCTL_METHOD_BUFFERED,
        &InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        &OutputBuffer,
        sizeof(OutputBuffer),
        &BytesReturned,
        NULL);

    if (!Result)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return false;
    }
    printf("    OutBuffer (%d): %s\n", BytesReturned, OutputBuffer);

    //
    // Performing METHOD_NIETHER
    //

    printf("\nCalling DeviceIoControl METHOD_NEITHER\n");

    strcpy_s(InputBuffer, "This String is from User Application; using METHOD_NEITHER");
    memset(OutputBuffer, 0, sizeof(OutputBuffer));

    Result = DeviceIoControl(Handle,
        (DWORD)IOCTL_SIOCTL_METHOD_NEITHER,
        &InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        &OutputBuffer,
        sizeof(OutputBuffer),
        &BytesReturned,
        NULL);

    if (!Result)
    {
        printf("Error in DeviceIoControl : %d\n", GetLastError());
        return false;
    }

    printf("    OutBuffer (%d): %s\n", BytesReturned, OutputBuffer);

    //
    // Performing METHOD_IN_DIRECT
    //

    printf("\nCalling DeviceIoControl METHOD_IN_DIRECT\n");

    strcpy_s(InputBuffer, "This String is from User Application; using METHOD_IN_DIRECT");
    strcpy_s(OutputBuffer, "This String is from User Application in OutBuffer; using METHOD_IN_DIRECT");

    Result = DeviceIoControl(Handle,
        (DWORD)IOCTL_SIOCTL_METHOD_IN_DIRECT,
        &InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        &OutputBuffer,
        sizeof(OutputBuffer),
        &BytesReturned,
        NULL);

    if (!Result)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return false;
    }

    printf("    Number of bytes transfered from OutBuffer: %d\n",
        BytesReturned);

    //
    // Performing METHOD_OUT_DIRECT
    //

    printf("\nCalling DeviceIoControl METHOD_OUT_DIRECT\n");

    strcpy_s(InputBuffer,  "This String is from User Application; using METHOD_OUT_DIRECT");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));

    Result = DeviceIoControl(Handle,
        (DWORD)IOCTL_SIOCTL_METHOD_OUT_DIRECT,
        &InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        &OutputBuffer,
        sizeof(OutputBuffer),
        &BytesReturned,
        NULL);

    if (!Result)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return false;
    }

    printf("    OutBuffer (%d): %s\n", BytesReturned, OutputBuffer);

    return true;
}

int
main()
{ 
    
	char* CpuId = (char*)malloc(13);
    PrintAppearance();

    GetCpuID(CpuId);

    printf("[*] The CPU Vendor is : %s \n", CpuId);

    if (std::string(CpuId) == "GenuineIntel")
    {
        printf("[*] The Processor virtualization technology is VT-x. \n");
    }
    else
    {
        printf("[*] This program is not designed to run in a non-VT-x environment !\n");
        return 1;
    }

    if (DetectVmxSupport())
    {
        printf("[*] VMX Operation is supported by your processor .\n");
    }
    else
    {
        printf("[*] VMX Operation is not supported by your processor .\n");
        return 1;
    }

    HANDLE hWnd = CreateFile(L"\\\\.\\Myhv",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ |
        FILE_SHARE_WRITE,
        NULL, /// lpSecurityAttirbutes
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL |
        FILE_FLAG_OVERLAPPED,
        NULL); /// lpTemplateFile

    if (hWnd == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed: %lu\n", GetLastError());
        std::cin.get();
        return 1;
    }

    TestIoctl(hWnd);
    CloseHandle(hWnd);
    std::cin.get();

    return 0;
}