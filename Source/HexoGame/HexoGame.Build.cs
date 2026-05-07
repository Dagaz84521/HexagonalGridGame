// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HexoGame : ModuleRules
{
	public HexoGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "HexGridCore" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput", "HexGridCore", "GameplayAbilities", "GameplayTags", "GameplayTasks" });
    }
}
