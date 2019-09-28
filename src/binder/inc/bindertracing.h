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
        Unknown = 0x0
    };

    class AssemblyBindEvent
    {
    public:
        AssemblyBindEvent(BINDER_SPACE::AssemblyName *assemblyName, const WCHAR *alc = nullptr);
        ~AssemblyBindEvent();

        void SetResult(BINDER_SPACE::Assembly *assembly);

    private:
        const EntryPoint m_entryPoint;
        bool m_success;
        PathString m_assemblyName;
        SString m_resultPath;
    };

    bool IsEnabled();
};

#endif // __BINDER_TRACING_H__
