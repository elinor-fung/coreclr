// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics.Tracing;
using System.Linq;
using System.Reflection;
using Xunit;

using Assert = Xunit.Assert;

namespace BinderTracingTests
{
    internal class BindEvent
    {
        internal AssemblyName AssemblyName;
        internal string AssemblyLoadContext;
        internal bool Success;

        internal bool Nested;
    }

    internal sealed class BinderEventListener : EventListener
    {
        public const EventKeywords TasksFlowActivityIds = (EventKeywords)0x80;
        public const EventKeywords BinderKeyword = (EventKeywords)0x4;
        public Dictionary<Guid, BindEvent> BindEvents = new Dictionary<Guid, BindEvent>();

        public IEnumerable<BindEvent> GetEventsBySimpleName(string name)
        {
            return BindEvents.Values.Where(e => !e.Nested && e.AssemblyName.Name == name);
        }

        protected override void OnEventSourceCreated(EventSource eventSource)
        {
            if (eventSource.Name == "Microsoft-Windows-DotNETRuntime")
            {
                EnableEvents(eventSource, EventLevel.Verbose, BinderKeyword);
            }
            else if (eventSource.Name == "System.Threading.Tasks.TplEventSource")
            {
                EnableEvents(eventSource, EventLevel.Verbose, TasksFlowActivityIds);
            }
        }

        protected override void OnEventWritten(EventWrittenEventArgs data)
        {
            if (data.EventSource.Name != "Microsoft-Windows-DotNETRuntime")
                return;

            object GetData(string name) => data.Payload[data.PayloadNames.IndexOf(name)];
            string GetDataString(string name) => GetData(name).ToString();

            switch (data.EventName)
            {
                case "AssemblyBindStart":
                    Assert.True(!BindEvents.ContainsKey(data.ActivityId), "AssemblyBindStart should not exist for same activity ID ");
                    var bindEvent = new BindEvent()
                    {
                        AssemblyName = new AssemblyName(GetDataString("AssemblyName")),
                        AssemblyLoadContext = GetDataString("AssemblyLoadContext"),
                        Nested = BindEvents.ContainsKey(data.RelatedActivityId)
                    };
                    BindEvents.Add(data.ActivityId, bindEvent);
                    break;
                case "AssemblyBindStop":
                    Assert.True(BindEvents.ContainsKey(data.ActivityId), "AssemblyBindStop should have a matching AssemblyBindStart");
                    BindEvents[data.ActivityId].Success = (bool)GetData("Success");
                    break;
            }
        }
    }
}
