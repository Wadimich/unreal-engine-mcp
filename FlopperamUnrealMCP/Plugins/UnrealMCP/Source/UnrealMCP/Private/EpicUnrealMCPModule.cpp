#include "EpicUnrealMCPModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEpicUnrealMCPModule"

void FEpicUnrealMCPModule::StartupModule()
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCP module started"));
}

void FEpicUnrealMCPModule::ShutdownModule()
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCP module stopped"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEpicUnrealMCPModule, UnrealMCP)
