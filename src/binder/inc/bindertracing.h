// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// bindertracing.h
//

#ifndef __BINDER_TRACING_H__
#define __BINDER_TRACING_H__

#include "bindertypes.hpp"

namespace BinderTracing
{
    enum EntryPoint
    {
        Unknown = 0,
        JIT = 1,
        Load = 2,
        LoadFromPath = 3,
        LoadFromStream = 4,
        Reflection = 5
    };

    class AssemblyBindEvent
    {
    public:
        AssemblyBindEvent(BINDER_SPACE::AssemblyName *assemblyName, const WCHAR *alc);
        AssemblyBindEvent(BINDER_SPACE::AssemblyName *assemblyName, OBJECTREF *managedALC);
        ~AssemblyBindEvent();

        void SetResult(BINDER_SPACE::Assembly *assembly);

    private:
        const EntryPoint m_entryPoint;
        bool m_success;
        PathString m_assemblyName;
        SString m_resultPath;
    };

    class AutoSetEntryPoint
    {
    public:
        AutoSetEntryPoint(EntryPoint entryPoint, const WCHAR *additionalData = nullptr);
        ~AutoSetEntryPoint();
    
    private:
        const EntryPoint m_entryPoint;
        const EntryPoint m_prevEntryPoint;
    };

    bool IsEnabled();
};

#endif // __BINDER_TRACING_H__
