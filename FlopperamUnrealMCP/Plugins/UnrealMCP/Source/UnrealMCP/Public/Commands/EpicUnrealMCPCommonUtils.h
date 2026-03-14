#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UBlueprint;

class UNREALMCP_API FEpicUnrealMCPCommonUtils
{
public:
    static TSharedPtr<FJsonObject> CreateErrorResponse(const FString& Message);
    static TSharedPtr<FJsonObject> CreateSuccessResponse(const TSharedPtr<FJsonObject>& Data = nullptr);
    static TSharedPtr<FJsonObject> CreateReadOnlyStubResponse(const FString& CommandType);

    static UBlueprint* LoadBlueprintByPath(const FString& BlueprintPath);
    static FString PinCategoryToString(const FEdGraphPinType& PinType);
};
