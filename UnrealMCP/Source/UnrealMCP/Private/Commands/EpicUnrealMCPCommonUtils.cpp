#include "Commands/EpicUnrealMCPCommonUtils.h"

#include "EditorAssetLibrary.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraphPin.h"

TSharedPtr<FJsonObject> FEpicUnrealMCPCommonUtils::CreateErrorResponse(const FString& Message)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), false);
    ResponseObject->SetStringField(TEXT("error"), Message);
    return ResponseObject;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPCommonUtils::CreateSuccessResponse(const TSharedPtr<FJsonObject>& Data)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), true);
    if (Data.IsValid())
    {
        ResponseObject->SetObjectField(TEXT("data"), Data);
    }
    return ResponseObject;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPCommonUtils::CreateReadOnlyStubResponse(const FString& CommandType)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), false);
    ResponseObject->SetStringField(TEXT("error"), TEXT("Command disabled in read-only UE4.27 integration"));
    ResponseObject->SetBoolField(TEXT("read_only"), true);
    ResponseObject->SetStringField(TEXT("command"), CommandType);
    return ResponseObject;
}

UBlueprint* FEpicUnrealMCPCommonUtils::LoadBlueprintByPath(const FString& BlueprintPath)
{
    return Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
}

FString FEpicUnrealMCPCommonUtils::PinCategoryToString(const FEdGraphPinType& PinType)
{
    FString Result = PinType.PinCategory.ToString();
    if (!PinType.PinSubCategory.IsNone())
    {
        Result += TEXT(":");
        Result += PinType.PinSubCategory.ToString();
    }
    if (PinType.PinSubCategoryObject.IsValid())
    {
        Result += TEXT(":");
        Result += PinType.PinSubCategoryObject->GetName();
    }
    return Result;
}
