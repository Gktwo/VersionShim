// Copyright © 2023 Xpl0itR
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// ReSharper disable once CppUnusedIncludeDirective

#include <stdio.h>
#include <windows.h>
#include "exports.h"
#include "util.h"

void OpenConsole();
BOOL APIENTRY DllMain(CONST HMODULE hModule, CONST DWORD fdwReason, CONST LPVOID lpvReserved)
{
    if (fdwReason != DLL_PROCESS_ATTACH)
        return TRUE;
    OpenConsole();
    LPSTR fileStr = NULL;
    DWORD fileLen = 0;

    if (!OpenReadFileUtf8("libraries.txt", &fileStr, &fileLen))
    {
        MessageBoxA(NULL, "Failed to read libraries.txt", PROJECT_NAME, ErrBoxType);
        return TRUE;
    }

    LPSTR callerPath[MAX_PATH + 1];
    if (!GetModuleFileNameA(NULL, callerPath, MAX_PATH + 1))  // NOLINT(clang-diagnostic-incompatible-pointer-types)
    {
        MessageBoxA(NULL, "Failed to read caller path", PROJECT_NAME, ErrBoxType);
        return TRUE;
    }

    int read, hasTarget = FALSE, targetMatched = FALSE;
    for (LPSTR line = SkipBomUtf8(fileStr), fileEnd = fileStr + fileLen; line < fileEnd; line += read)
    {
        read = TerminateLine(line);

        if (*line == '#')
            continue;

        if (*line == '*')
        {
            if (!targetMatched)
            {
                hasTarget = TRUE;

                if (StrEndsWith(callerPath, line + 1))  // NOLINT(clang-diagnostic-incompatible-pointer-types)
                {
                    targetMatched = TRUE;
                }
            }

            continue;
        }

        if (hasTarget && !targetMatched)
            return TRUE;

        LPSTR commaPos = strchr(line, ',');
        if (commaPos != NULL)
        {
            *commaPos = '\0';               // 分割字符串
            int delay = atoi(commaPos + 1); // 获取延迟时间
            printf("[*] Loading %s with delay %d ms\n", line, delay);
            Sleep(delay);                   // 等待指定的时间
        }


        // ReSharper disable once CppPointerToIntegralConversion
        if (!LoadLibraryA(line))
        {
            MessageBoxA(NULL, line, PROJECT_NAME" - Failed to load library", ErrBoxType);
        }
        else
        {
            printf("Loaded %s\n", line);
        }
    }

    HeapFree(GetProcessHeap(), 0, fileStr);
    return TRUE;
}

void OpenConsole()
{
    // 分配控制台
    AllocConsole();

    // 重定向标准输入、输出和错误流到控制台
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    // 设置控制台窗口大小
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SMALL_RECT windowSize = { 0, 0, 120, 30 };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    // 显示控制台窗口
    ShowWindow(GetConsoleWindow(), SW_SHOW);
}