newaction {
    trigger = "clean",
    description = "Remove all binaries, intermediate binaries, and Visual Studio files.",
    execute = function()
        print("Removing binaries, intermediate binaries and project files...")
        os.rmdir("build")
        os.rmdir(".vs")
        os.execute("del /s /q *.sln *.vcxproj *.vcxproj.filters *.vcxproj.user 2>nul")
        print("Done cleaning!")
    end
}

workspace "GoFlyKiller"
    location "build"
    configurations {"Debug", "Release"}
    architecture "x64" 

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "GoFlyKiller"
    kind "ConsoleApp" 
    language "C++"
    cppdialect "C++17"
    characterset "MBCS" 

    files {
        "src/**.cpp",  
    }

    links {
         "wininet",
         "winhttp",
         "psapi"
    }

    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/tmp/" .. outputdir .. "/%{prj.name}")

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"