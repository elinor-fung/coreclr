// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Linq;
using System.Reflection;
using System.Runtime.Loader;

using Assert = Xunit.Assert;

namespace BinderTracingTests
{
    class BinderTracingTest
    {
        private const int EventWaitTimeoutInMilliseconds = 1000;

        public static int Main(string [] args)
        {
            using (var listener = new BinderEventListener())
            {
                string assemblyName = "System.Xml";
                Assembly asm = Assembly.Load(assemblyName);

                var events = listener.WaitForEventsForAssembly(assemblyName, EventWaitTimeoutInMilliseconds);
                Assert.True(events.Count() == 1, $"Bind event count for {assemblyName} - expected: 1, actual: {events.Count()}");
                BindEvent bindEvent = events.First();
                Assert.True(bindEvent.AssemblyLoadContext == "Default", $"ALC for {assemblyName} bind - expected: Default, actual {bindEvent.AssemblyLoadContext}");
                Assert.True(bindEvent.Success, $"Expected bind for {assemblyName} to succeed");
            }

            using (var listener = new BinderEventListener())
            {
                string alcName = "CustomName";
                string assemblyName = "DoesNotExist";
                AssemblyLoadContext alc = new AssemblyLoadContext(alcName);
                try
                {
                    alc.LoadFromAssemblyName(new AssemblyName(assemblyName));
                }
                catch { }

                var events = listener.WaitForEventsForAssembly(assemblyName, EventWaitTimeoutInMilliseconds);
                Assert.True(events.Count() == 1, $"Expected one bind event for {assemblyName}");
                BindEvent bindEvent = events.First();
                Assert.True(bindEvent.AssemblyLoadContext.Contains(alcName) && bindEvent.AssemblyLoadContext.Contains("System.Runtime.Loader.AssemblyLoadContext"), $"Expected bind event for {assemblyName} to be in ALC {alcName}");
                Assert.False(bindEvent.Success, $"Expected bind for {assemblyName} to fail");
            }

            return 100;
        }
    }
}
