#include "MCPServerRunnable.h"

#include "EpicUnrealMCPBridge.h"
#include "Dom/JsonObject.h"
#include "HAL/PlatformProcess.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SocketSubsystem.h"

FMCPServerRunnable::FMCPServerRunnable(UEpicUnrealMCPBridge* InBridge, TSharedPtr<FSocket> InListenerSocket)
    : Bridge(InBridge)
    , ListenerSocket(InListenerSocket)
    , bRunning(true)
{
}

FMCPServerRunnable::~FMCPServerRunnable()
{
}

bool FMCPServerRunnable::Init()
{
    return true;
}

uint32 FMCPServerRunnable::Run()
{
    while (bRunning)
    {
        bool bPending = false;
        if (ListenerSocket.IsValid() && ListenerSocket->HasPendingConnection(bPending) && bPending)
        {
            ClientSocket = MakeShareable(ListenerSocket->Accept(TEXT("MCPClient")));
            if (ClientSocket.IsValid())
            {
                ClientSocket->SetNoDelay(true);
                const int32 SocketBufferSize = 65536;
                int32 OutSize = 0;
                ClientSocket->SetSendBufferSize(SocketBufferSize, OutSize);
                ClientSocket->SetReceiveBufferSize(SocketBufferSize, OutSize);

                uint8 Buffer[8192];
                while (bRunning)
                {
                    int32 BytesRead = 0;
                    if (ClientSocket->Recv(Buffer, sizeof(Buffer) - 1, BytesRead))
                    {
                        if (BytesRead == 0)
                        {
                            break;
                        }

                        Buffer[BytesRead] = '\0';
                        const FString ReceivedText = UTF8_TO_TCHAR(reinterpret_cast<const char*>(Buffer));

                        TSharedPtr<FJsonObject> JsonObject;
                        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedText);
                        if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
                        {
                            continue;
                        }

                        FString CommandType;
                        if (!JsonObject->TryGetStringField(TEXT("type"), CommandType))
                        {
                            continue;
                        }

                        TSharedPtr<FJsonObject> Params;
                        if (JsonObject->HasTypedField<EJson::Object>(TEXT("params")))
                        {
                            Params = JsonObject->GetObjectField(TEXT("params"));
                        }
                        else
                        {
                            Params = MakeShared<FJsonObject>();
                        }

                        const FString Response = Bridge->ExecuteCommand(CommandType, Params);
                        FTCHARToUTF8 UTF8Response(*Response);
                        int32 TotalBytesSent = 0;
                        const int32 TotalDataSize = UTF8Response.Length();
                        const uint8* DataToSend = reinterpret_cast<const uint8*>(UTF8Response.Get());

                        while (TotalBytesSent < TotalDataSize)
                        {
                            int32 BytesSent = 0;
                            if (!ClientSocket->Send(DataToSend + TotalBytesSent, TotalDataSize - TotalBytesSent, BytesSent))
                            {
                                break;
                            }
                            TotalBytesSent += BytesSent;
                        }
                    }
                    else
                    {
                        const int32 LastError = (int32)ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
                        if (LastError == SE_EWOULDBLOCK || LastError == SE_EINTR)
                        {
                            FPlatformProcess::Sleep(0.01f);
                            continue;
                        }
                        break;
                    }
                }
            }
        }

        FPlatformProcess::Sleep(0.1f);
    }

    return 0;
}

void FMCPServerRunnable::Stop()
{
    bRunning = false;
}

void FMCPServerRunnable::Exit()
{
}
