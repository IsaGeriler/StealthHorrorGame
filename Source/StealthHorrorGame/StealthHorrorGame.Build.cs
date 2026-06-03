// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StealthHorrorGame : ModuleRules
{
	public StealthHorrorGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"StealthHorrorGame",
			"StealthHorrorGame/Variant_Horror",
			"StealthHorrorGame/Variant_Horror/UI",
			"StealthHorrorGame/Variant_Shooter",
			"StealthHorrorGame/Variant_Shooter/AI",
			"StealthHorrorGame/Variant_Shooter/UI",
			"StealthHorrorGame/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
