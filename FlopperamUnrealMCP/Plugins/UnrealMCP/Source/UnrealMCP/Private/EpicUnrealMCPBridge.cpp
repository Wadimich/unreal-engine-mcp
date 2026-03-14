#include "EpicUnrealMCPBridge.h"

#include "Async/Async.h"
#include "Commands/EpicUnrealMCPBlueprintCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "MCPServerRunnable.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SocketSubsystem.h"

#define MCP_SERVER_HOST "127.0.0.1"
#define MCP_SERVER_PORT 55557

UEpicUnrealMCPBridge::UEpicUnrealMCPBridge()
    : bIsRunning(false)
    , ServerThread(nullptr)
    , Port(MCP_SERVER_PORT)
{
    FIPv4Address::Parse(TEXT(MCP_SERVER_HOST), ServerAddress);
    BlueprintCommands = MakeShared<FEpicUnrealMCPBlueprintCommands>();
}

UEpicUnrealMCPBridge::~UEpicUnrealMCPBridge()
{
    BlueprintCommands.Reset();
}

void UEpicUnrealMCPBridge::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Display, TEXT("UnrealMCP bridge initializing"));
    StartServer();
}

void UEpicUnrealMCPBridge::Deinitialize()
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCP bridge shutting down"));
    StopServer();
    Super::Deinitialize();
}

void UEpicUnrealMCPBridge::StartServer()
{
    if (bIsRunning)
    {
        return;
    }

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCP: Failed to get socket subsystem"));
        return;
    }

    TSharedPtr<FSocket> NewListenerSocket = MakeShareable(SocketSubsystem->CreateSocket(NAME_Stream, TEXT("UnrealMCPListener"), false));
    if (!NewListenerSocket.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCP: Failed to create listener socket"));
        return;
    }

    NewListenerSocket->SetReuseAddr(true);
    NewListenerSocket->SetNonBlocking(true);

    FIPv4Endpoint Endpoint(ServerAddress, Port);
    if (!NewListenerSocket->Bind(*Endpoint.ToInternetAddr()))
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCP: Failed to bind listener socket to %s:%d"), *ServerAddress.ToString(), Port);
        return;
    }

    if (!NewListenerSocket->Listen(5))
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCP: Failed to start listening"));
        return;
    }

    ListenerSocket = NewListenerSocket;
    bIsRunning = true;
    ServerThread = FRunnableThread::Create(new FMCPServerRunnable(this, ListenerSocket), TEXT("UnrealMCPServerThread"), 0, TPri_Normal);

    if (!ServerThread)
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCP: Failed to create server thread"));
        StopServer();
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("UnrealMCP: Server started on %s:%d"), *ServerAddress.ToString(), Port);
}

void UEpicUnrealMCPBridge::StopServer()
{
    if (!bIsRunning)
    {
        return;
    }

    bIsRunning = false;

    if (ServerThread)
    {
        ServerThread->Kill(true);
        delete ServerThread;
        ServerThread = nullptr;
    }

    if (ConnectionSocket.IsValid())
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket.Get());
        ConnectionSocket.Reset();
    }

    if (ListenerSocket.IsValid())
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket.Get());
        ListenerSocket.Reset();
    }
}

FString UEpicUnrealMCPBridge::ExecuteCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    TPromise<FString> Promise;
    TFuture<FString> Future = Promise.GetFuture();

    AsyncTask(ENamedThreads::GameThread, [this, CommandType, Params, Promise = MoveTemp(Promise)]() mutable
    {
        TSharedPtr<FJsonObject> ResultJson;

        if (CommandType == TEXT("ping"))
        {
            ResultJson = MakeShared<FJsonObject>();
            ResultJson->SetBoolField(TEXT("success"), true);
            ResultJson->SetStringField(TEXT("message"), TEXT("pong"));
        }
        else
        {
            ResultJson = BlueprintCommands->HandleCommand(CommandType, Params);
        }

        FString ResponseString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
        FJsonSerializer::Serialize(ResultJson.ToSharedRef(), Writer);
        Promise.SetValue(ResponseString);
    });

    return Future.Get();
}
