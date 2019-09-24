// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
// ============================================================
//
// bindertracing.cpp
//


//
// Implements helpers for binder tracing
//
// ============================================================


#include "common.h"
#include "bindertracing.h"

#include "assemblyname.hpp"

#ifdef FEATURE_EVENT_TRACE
#include "eventtracebase.h"
#endif // FEATURE_EVENT_TRACE

using namespace BINDER_SPACE;

namespace
{
    thread_local GUID s_RequestId = GUID_NULL;
    thread_local uint32_t s_RequestDepth = 0;

    const WCHAR *s_activityName = W("AssemblyBind");
}

const GUID& BinderTracing::GetCurrentRequestId()
{
    return s_RequestId;
}

void BinderTracing::AssemblyBindStart(AssemblyName *pAssemblyName, uint16_t entryPointId)
{
#ifdef FEATURE_EVENT_TRACE
    if (!EventEnabledAssemblyBindStart())
        return;

    GUID activityId = GUID_NULL;
    GUID relatedActivityId = GUID_NULL;
    {
        GCX_COOP();
        struct _gc {
            OBJECTREF activityTracker;
            STRINGREF providerName;
            STRINGREF activityName;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        GCPROTECT_BEGIN(gc);

        MethodDescCallSite getInstance(METHOD__ACTIVITY_TRACKER__GET_INSTANCE);
        gc.activityTracker = getInstance.Call_RetOBJECTREF(NULL);

        gc.providerName = StringObject::NewString(MICROSOFT_WINDOWS_DOTNETRUNTIME_PRIVATE_PROVIDER_EVENTPIPE_Context.Name);
        gc.activityName = StringObject::NewString(s_activityName);

        MethodDescCallSite onStart(METHOD__ACTIVITY_TRACKER__ON_START, &gc.activityTracker);
        ARG_SLOT args[] =
        {
            ObjToArgSlot(gc.activityTracker),
            ObjToArgSlot(gc.providerName),
            ObjToArgSlot(gc.activityName),
            0,
            PtrToArgSlot(&activityId),
            PtrToArgSlot(&relatedActivityId),
        };
        onStart.Call(args);

        GCPROTECT_END();
    }

    _ASSERTE(pAssemblyName != nullptr);

    PathString assemblyDisplayName;
    pAssemblyName->GetDisplayName(assemblyDisplayName, AssemblyName::INCLUDE_VERSION);

    if (IsEqualGUID(s_RequestId, GUID_NULL))
        CoCreateGuid(&s_RequestId);

    s_RequestDepth++;
    FireEtwAssemblyBindStart(&s_RequestId, GetClrInstanceId(), entryPointId, assemblyDisplayName.GetUnicode(), &activityId, &relatedActivityId);
#endif // FEATURE_EVENT_TRACE
}

void BinderTracing::AssemblyBindEnd(AssemblyName *pAssemblyName, uint16_t entryPointId)
{
#ifdef FEATURE_EVENT_TRACE
    if (!EventEnabledAssemblyBindEnd())
        return;

    GUID activityId = GUID_NULL;
    {
        GCX_COOP();
        struct _gc {
            OBJECTREF activityTracker;
            STRINGREF providerName;
            STRINGREF activityName;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        GCPROTECT_BEGIN(gc);

        MethodDescCallSite getInstance(METHOD__ACTIVITY_TRACKER__GET_INSTANCE);
        gc.activityTracker = getInstance.Call_RetOBJECTREF(NULL);

        gc.providerName = StringObject::NewString(MICROSOFT_WINDOWS_DOTNETRUNTIME_PRIVATE_PROVIDER_EVENTPIPE_Context.Name);
        gc.activityName = StringObject::NewString(s_activityName);

        MethodDescCallSite onStop(METHOD__ACTIVITY_TRACKER__ON_STOP, &gc.activityTracker);
        ARG_SLOT args[] =
        {
            ObjToArgSlot(gc.activityTracker),
            ObjToArgSlot(gc.providerName),
            ObjToArgSlot(gc.activityName),
            0,
            PtrToArgSlot(&activityId),
        };
        onStop.Call(args);

        GCPROTECT_END();
    }

    _ASSERTE(!IsEqualGUID(s_RequestId, GUID_NULL));

    PathString assemblyDisplayName;
    if (pAssemblyName != nullptr)
        pAssemblyName->GetDisplayName(assemblyDisplayName, AssemblyName::INCLUDE_VERSION);

    s_RequestDepth--;
    FireEtwAssemblyBindEnd(&s_RequestId, GetClrInstanceId(), entryPointId, assemblyDisplayName.GetUnicode(), &activityId);

    if (s_RequestDepth == 0)
        s_RequestId = GUID_NULL;
#endif // FEATURE_EVENT_TRACE
}