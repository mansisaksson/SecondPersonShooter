#include "SecondPersonShooter.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SecondPersonShooter, "SecondPersonShooter" );
//General Log
DEFINE_LOG_CATEGORY(DebugLog);

//Logging during game startup
DEFINE_LOG_CATEGORY(DebugInit);

//Logging for your AI system
DEFINE_LOG_CATEGORY(DebugAI);

//Logging for Critical Errors that must always be addressed
DEFINE_LOG_CATEGORY(DebugError);

ADefaultGameMode* SPS::GetGameMode(UObject* WorldContextObject)
{
	return Cast<ADefaultGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
}