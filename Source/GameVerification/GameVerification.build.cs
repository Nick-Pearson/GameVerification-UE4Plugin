// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class GameVerification : ModuleRules
	{
		public GameVerification(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateIncludePaths.AddRange(new string[] {
					"GameVerification/Private",
				});

			PublicDependencyModuleNames.AddRange(new string[] {
					"Core",
					"GameVerificationLibrary",

				});

			PrivateDependencyModuleNames.AddRange(new string[] {
					"CoreUObject",
					"Engine",
					"Projects",
                    "UnrealEd"
                });

			string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", "GameVerificationLibrary"));

			// Include headers
			PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));
		}
	}
}
