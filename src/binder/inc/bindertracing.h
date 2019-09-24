// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


#ifndef __BINDER_TRACING_H__
#define __BINDER_TRACING_H__

#include "bindertypes.hpp"

namespace BinderTracing
{
    void AssemblyBindStart(BINDER_SPACE::AssemblyName *pAssemblyName, uint16_t entryPointId);
    void AssemblyBindEnd(BINDER_SPACE::AssemblyName *pAssemblyName, uint16_t entryPointId);

    const GUID& GetCurrentRequestId();
};

#endif // __BINDER_TRACING_H__
