-- premake5.lua

workspace "CScripter"
    configurations { 
        "Debug",
        "Release"
    }

    platforms {
        "x64"
    }

flags { "MultiProcessorCompile" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "CScripter"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    characterset "MBCS"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/" .. outputdir .. "/bin-int/")

    files { 
        "src/**.h", 
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs {
        "include/",
        "src/"
    }

    libdirs { 
        "lib/"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "platforms:x64"
        architecture "x86_64"
        links {
            "kernel32",
            "crypt32",
            "Wldap32",
            "Normaliz",
            "libtcc"
        }