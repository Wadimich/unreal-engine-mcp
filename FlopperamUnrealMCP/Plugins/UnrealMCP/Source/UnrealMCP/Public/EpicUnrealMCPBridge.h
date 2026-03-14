#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "EpicUnrealMCPBridge.generated.h"

class FRunnableThread;
class FMCPServerRunnable;
class FEpicUnrealMCPBlueprintCommands;
class FSocket;

UCLASS()
class UNREALMCP_API UEpicUnrealMCPBridge : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    UEpicUnrealMCPBridge();
    virtual ~UEpicUnrealMCPBridge();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void StartServer();
    void StopServer();
    bool IsRunning() const { return bIsRunning; }

    FString ExecuteCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    bool bIsRunning;
    TSharedPtr<FSocket> ListenerSocket;
    TSharedPtr<FSocket> ConnectionSocket;
    FRunnableThread* ServerThread;
    FIPv4Address ServerAddress;
    uint16 Port;

    TSharedPtr<FEpicUnrealMCPBlueprintCommands> BlueprintCommands;
};
