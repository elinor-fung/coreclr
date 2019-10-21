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

#include "assembly.hpp"
#include "assemblyname.hpp"

#ifdef FEATURE_EVENT_TRACE
#include "eventtracebase.h"
#endif // FEATURE_EVENT_TRACE

using namespace BINDER_SPACE;

namespace
{
    thread_local uint32_t s_RequestDepth = 0;
    thread_local BinderTracing::EntryPoint s_EntryPoint;

    const WCHAR *s_activityName = W("AssemblyBind");

    void FireAssemblyBindStart(const WCHAR *name, BinderTracing::EntryPoint entryPointId, const WCHAR *alc)
    {
#ifdef FEATURE_EVENT_TRACE
        if (!EventEnabledAssemblyBindStart())
            return;

        GUID activityId = GUID_NULL;
        GUID relatedActivityId = GUID_NULL;
        bool started = false;
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
            started = onStart.Call_RetBool(args);

            GCPROTECT_END();
        }

        s_RequestDepth++;
        if (started)
        {
            FireEtwAssemblyBindStart(GetClrInstanceId(), name, entryPointId, alc, &activityId, &relatedActivityId);
        }
#endif // FEATURE_EVENT_TRACE
    }

    void FireAssemblyBindStart(const WCHAR *name, BinderTracing::EntryPoint entryPointId, OBJECTREF *managedALC)
    {
#ifdef FEATURE_EVENT_TRACE
        if (!EventEnabledAssemblyBindStart())
            return;

        SString alcName;
        {
            GCX_COOP();
            struct _gc {
                STRINGREF alcName;
            } gc;
            ZeroMemory(&gc, sizeof(gc));

            GCPROTECT_BEGIN(gc);

            MethodDescCallSite toString(METHOD__OBJECT__TO_STRING, managedALC);
            ARG_SLOT args[] = { ObjToArgSlot(*managedALC) };
            gc.alcName = toString.Call_RetSTRINGREF(args);
            gc.alcName->GetSString(alcName);

            GCPROTECT_END();
        }

        FireAssemblyBindStart(name, entryPointId, alcName.GetUnicode());
#endif // FEATURE_EVENT_TRACE
    }

    void FireAssemblyBindStop(const WCHAR *name, BinderTracing::EntryPoint entryPointId, bool success, const WCHAR *resultPath)
    {
#ifdef FEATURE_EVENT_TRACE
        if (!EventEnabledAssemblyBindStop())
            return;

        GUID activityId = GUID_NULL;
        bool stopped = false;
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
            stopped = onStop.Call_RetBool(args);

            GCPROTECT_END();
        }

        s_RequestDepth--;

        if (stopped)
        {
            FireEtwAssemblyBindStop(GetClrInstanceId(), name, entryPointId, success, resultPath, &activityId);
        }
#endif // FEATURE_EVENT_TRACE
    }
}

bool BinderTracing::IsEnabled()
{
#ifdef FEATURE_EVENT_TRACE
    // Just check for the AssemblyBindStart event being enabled.
    return EventEnabledAssemblyBindStart();
#endif // FEATURE_EVENT_TRACE
    return false;
}

namespace BinderTracing
{
    AssemblyBindEvent::AssemblyBindEvent(AssemblyName *assemblyName, const WCHAR *alc)
        : m_entryPoint { s_EntryPoint }
        , m_success { false }
    {
        _ASSERTE(assemblyName != nullptr);

        assemblyName->GetDisplayName(m_assemblyName, AssemblyName::INCLUDE_VERSION);
        FireAssemblyBindStart(m_assemblyName.GetUnicode(), m_entryPoint, alc);
    }

    AssemblyBindEvent::AssemblyBindEvent(AssemblyName *assemblyName, OBJECTREF *managedALC)
        : m_entryPoint { s_EntryPoint }
        , m_success { false }
    {
        _ASSERTE(assemblyName != nullptr);

        assemblyName->GetDisplayName(m_assemblyName, AssemblyName::INCLUDE_VERSION);
        FireAssemblyBindStart(m_assemblyName.GetUnicode(), m_entryPoint, managedALC);
    }

    AssemblyBindEvent::~AssemblyBindEvent()
    {
        FireAssemblyBindStop(m_assemblyName.GetUnicode(), m_entryPoint, m_success, m_resultPath.GetUnicode());
    }

    void AssemblyBindEvent::SetResult(BINDER_SPACE::Assembly *assembly)
    {
        m_success = assembly != nullptr;
        if (m_success)
            m_resultPath = assembly->GetPEImage()->GetPath();
    }

    AutoSetEntryPoint::AutoSetEntryPoint(EntryPoint entryPoint, const WCHAR *additionalData)
        : m_entryPoint { entryPoint }
        , m_prevEntryPoint { s_EntryPoint }
    {
        s_EntryPoint = m_entryPoint;
    }

    AutoSetEntryPoint::~AutoSetEntryPoint()
    {
        _ASSERTE(s_EntryPoint == m_entryPoint);
        s_EntryPoint = m_prevEntryPoint;
    }
}
