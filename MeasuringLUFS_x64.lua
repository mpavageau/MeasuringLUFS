-- A solution contains projects, and defines the available configurations
solution "MeasuringLUFS_x64"
    configurations { "Debug", "Release" }
if ( _ACTION == "vs2012") then
    location "build/vs2012"
elseif ( _ACTION == "xcode4") then
    location "build/xcode4"
else
    location "build/todo_set_platform"
end
    flags { "WinMain", "StaticRuntime", "Unicode", "NoRuntimeChecks" }
    includedirs { 
        "source", 
        "extern/juce", 
        "extern/asiosdk/common", 
        "extern/vstsdk2.4", 
        }
    files { 
        "source/**.h", 
        "source/**.cpp",
        "extern/juce/modules/juce_audio_basics/juce_audio_basics.cpp",
        "extern/juce/modules/juce_audio_devices/juce_audio_devices.cpp",
        "extern/juce/modules/juce_audio_formats/juce_audio_formats.cpp",
        "extern/juce/modules/juce_audio_processors/juce_audio_processors.cpp",
        "extern/juce/modules/juce_audio_utils/juce_audio_utils.cpp",
        "extern/juce/modules/juce_core/juce_core.cpp",
        "extern/juce/modules/juce_cryptography/juce_cryptography.cpp",
        "extern/juce/modules/juce_data_structures/juce_data_structures.cpp",
        "extern/juce/modules/juce_events/juce_events.cpp",
        "extern/juce/modules/juce_graphics/juce_graphics.cpp",
        "extern/juce/modules/juce_gui_basics/juce_gui_basics.cpp",
        "extern/juce/modules/juce_gui_extra/juce_gui_extra.cpp",
        }
    objdir "build/temp"
    platforms "x64"
    targetdir "build/bin"
    --links { "user32", "gdi32" } 
    
    
    project "MeasuringLUFS_App_x64"
        kind "WindowedApp"
        language "C++"
        defines { 
            "MEASURING_LUFS_APPLICATION",
            "USING_ASIO", -- comment this out not to use ASIO 
        }
     
        configuration "Debug"
            defines "DEBUG"
            flags { "Symbols", "ExtraWarnings", }

        configuration "Release"
            defines "NDEBUG"
            flags "Optimize" 
            
    project "MeasuringLUFS_Plug_x64"
        kind "SharedLib"
        language "C++"
        defines { 
            "MEASURING_LUFS_PLUGIN",
        }
        configuration "Debug"
            defines "DEBUG"
            flags "Symbols"

        configuration "Release"
            defines "NDEBUG"
            flags "Optimize" 

            
		 