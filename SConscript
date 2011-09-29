import os, shutil, datetime
import Version

Import("env")

if env["SCONS_STAGE"] == "build" :

   myenv = env.Clone()
   myenv.MergeFlags(env["SWIFTEN_FLAGS"])
   myenv.MergeFlags(env["SWIFTEN_DEP_FLAGS"])

   sources = [
           "main.cpp",
	   "ActiveSessionPair.cpp",
	   "StaticDomainNameResolver.cpp",
	   "IdleSession.cpp",
	   "LatencyWorkloadBenchmark.cpp",
    	   "ThreadSafeNetworkFactories.cpp",
      ]
   myenv.Program("xmppench", sources)