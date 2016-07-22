#include "SecondPersonShooter.h"
#include "Debug.h"

void UDebug::LogOnScreen(FString message)
{
	LogOnScreen(message, 2.5f, FColor::White);
}

void UDebug::LogOnScreen(FString message, FColor color)
{
	LogOnScreen(message, 2.5f, color);
}

void UDebug::LogOnScreen(FString message, float screenTime, FColor color)
{
	GEngine->AddOnScreenDebugMessage(-1, screenTime, color, message);
}

void UDebug::Log(FString message)
{
	UE_LOG(DebugLog, Log, TEXT("%s"), *message);
}

void UDebug::LogWarning(FString message)
{
	UE_LOG(DebugLog, Warning, TEXT("%s"), *message);
}

void UDebug::LogFatalError(FString message)
{
	UE_LOG(DebugError, Error, TEXT("%s"), *message);
	//UKismetSystemLibrary::QuitGame(GetWorld(), )
}
