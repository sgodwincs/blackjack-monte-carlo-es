---- Workspace: GameDev-Practice ----

workspace("Blackjack MC")
configurations({
    "Debug32",
    "Release32",
    "Debug64",
    "Release64"
})
platforms({
    "Windows"
})

characterset("Unicode")
language("C++")
location("generated")
targetdir("build/bin/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}")
objdir("build/obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}")

filter("configurations:Debug*")
defines("PRJ_DEBUG")
symbols("On")

filter("configurations:Release*")
defines({
    "PRJ_RELEASE",
    "NDEBUG"
})
flags("LinkTimeOptimization")
optimize("On")

filter("configurations:*32")
defines("PRJ_X86")
architecture("x86")

filter("configurations:*64")
defines("PRJ_X86_64")
architecture("x86_64")

filter("platforms:Windows")
defines("PRJ_WINDOWS")
system("windows")

filter({})

-- Configurations/Flags

flags("C++14")

filter("platforms:Windows")
buildoptions("/std:c++latest")
forceincludes("pch.hpp")

filter({})

---- Project: Blackjack MC ----

-- Settings

project("Blackjack MC")
kind("ConsoleApp")

debugdir("blackjack-mc")

includedirs("blackjack-mc/header")
files("blackjack-mc/source/**.cpp")

pchheader("pch.hpp")
pchsource("blackjack-mc/source/pch.cpp")
