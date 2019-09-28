// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//
// A simple CoreCLR host that runs a managed binary with the same name as this executable but with *.dll extension
// The dll binary must contain main entry point.
//

#include <windows.h>
#include <stdio.h>
#include "HostEnvironment.h"
#include "error.h"

bool HostEnvironment::Setup()
{
    // Discover the path to this exe's module. All other files are expected to be in the same directory.
    DWORD hostPathLength = ::GetModuleFileNameA(::GetModuleHandleA(nullptr), m_hostPath, MAX_LONGPATH);

    if (hostPathLength == MAX_LONGPATH)
    {
        error("Host path is too long.");
        return false;
    }

    // Search for the last backslash in the host path.
    int lastBackslashIndex;
    for (lastBackslashIndex = hostPathLength - 1; lastBackslashIndex >= 0; lastBackslashIndex--) {
        if (m_hostPath[lastBackslashIndex] == '\\') {
            break;
        }
    }

    // Copy the directory path
    strncpy(m_hostDir, m_hostPath, lastBackslashIndex + 1);
    m_hostDir[lastBackslashIndex + 1] = 0;

    // Build the DLL name
    strncpy(m_appPath, m_hostPath, hostPathLength + 1);

    auto extension = strchr(m_appPath, '.');
    if (extension == NULL || (strcmp(extension, ".exe") != 0)) {
        error("This executable needs to have 'exe' extension.");
        return false;
    }
    // Change the extension from ".exe" to ".dll"
    extension[1] = 'd';
    extension[2] = 'l';
    extension[3] = 'l';

    m_Tpa.Compute(m_hostDir);

    return true;
}

