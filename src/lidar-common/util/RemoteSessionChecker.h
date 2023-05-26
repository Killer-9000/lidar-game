#pragma once
//
//#if WIN32
//
//#include <windows.h>
//#pragma comment(lib, "user32.lib")
//
//#define TERMINAL_SERVER_KEY "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\"
//#define GLASS_SESSION_ID    "GlassSessionId"
//
//bool IsCurrentSessionRemoteable()
//{
//    bool fIsRemoteable = false;
//                                       
//    if (GetSystemMetrics(SM_REMOTESESSION)) 
//    {
//        fIsRemoteable = true;
//    }
//    else
//    {
//        HKEY hRegKey = nullptr;
//        LONG lResult;
//
//        lResult = RegOpenKeyEx(
//            HKEY_LOCAL_MACHINE,
//            TERMINAL_SERVER_KEY,
//            0, // ulOptions
//            KEY_READ,
//            &hRegKey
//            );
//
//        if (lResult == ERROR_SUCCESS)
//        {
//            DWORD dwGlassSessionId;
//            DWORD cbGlassSessionId = sizeof(dwGlassSessionId);
//            DWORD dwType;
//
//            lResult = RegQueryValueEx(
//                hRegKey,
//                GLASS_SESSION_ID,
//                nullptr, // lpReserved
//                &dwType,
//                (BYTE*) &dwGlassSessionId,
//                &cbGlassSessionId
//                );
//
//            if (lResult == ERROR_SUCCESS)
//            {
//                DWORD dwCurrentSessionId;
//
//                if (ProcessIdToSessionId(GetCurrentProcessId(), &dwCurrentSessionId))
//                {
//                    fIsRemoteable = (dwCurrentSessionId != dwGlassSessionId);
//                }
//            }
//        }
//
//        if (hRegKey)
//        {
//            RegCloseKey(hRegKey);
//        }
//    }
//
//    return fIsRemoteable;
//}
//
//#endif
