// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SecondPersonShooter : ModuleRules
{
	public SecondPersonShooter(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "AIModule", "OnlineSubsystem"});
        //PrivateDependencyModuleNames.Add("OnlineSubsystem");
    }
}
