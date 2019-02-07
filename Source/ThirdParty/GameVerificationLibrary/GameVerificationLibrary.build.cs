// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class GameVerificationLibrary : ModuleRules
{
    public GameVerificationLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string BaseDirectory = Path.GetFullPath(ModuleDirectory);

        // Include headers
        PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string lib = Path.Combine(BaseDirectory, "Win64");

            // Add the import library
            PublicLibraryPaths.Add(lib);
            PublicAdditionalLibraries.Add(Path.Combine(lib, "VerificationRuntime.lib"));

            // Dynamic
            RuntimeDependencies.Add(Path.Combine(lib, "VerificationRuntime.dll"));
            PublicDelayLoadDLLs.Add("VerificationRuntime.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string lib = Path.Combine(BaseDirectory, "Linux");

            // Add the import library
            PublicLibraryPaths.Add(lib);
            PublicAdditionalLibraries.Add(Path.Combine(lib, "libVerificationRuntime.so"));
            RuntimeDependencies.Add(Path.Combine(lib, "libVerificationRuntime.so"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string lib = Path.Combine(BaseDirectory, "Mac");

            // Add the import library
            PublicLibraryPaths.Add(lib);
            PublicAdditionalLibraries.Add(Path.Combine(lib, "libVerificationRuntime.dylib"));
            RuntimeDependencies.Add(Path.Combine(lib, "libVerificationRuntime.dylib"));
        }
    }
}