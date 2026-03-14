#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Sockets.h"

class UEpicUnrealMCPBridge;

class FMCPServerRunnable : public FRunnable
{
public:
    FMCPServerRunnable(UEpicUnrealMCPBridge* InBridge, TSharedPtr<FSocket> InListenerSocket);
    virtual ~FMCPServerRunnable();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

private:
    UEpicUnrealMCPBridge* Bridge;
    TSharedPtr<FSocket> ListenerSocket;
    TSharedPtr<FSocket> ClientSocket;
    bool bRunning;
};
