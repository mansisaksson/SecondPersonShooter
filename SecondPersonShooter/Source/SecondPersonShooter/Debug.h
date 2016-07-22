#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Debug.generated.h"

UCLASS()
class SECONDPERSONSHOOTER_API UDebug : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void LogOnScreen(FString message);
	static void LogOnScreen(FString message, FColor color);
	static void LogOnScreen(FString message, float screenTime, FColor color = FColor::White);

	static void Log(FString message);
	static void LogWarning(FString message);
	static void LogFatalError(FString message);

private:
	UDebug() {}
	
	
};
