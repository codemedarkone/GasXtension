// Copyright Epic Games, Inc.

#include "GasXSchemaParser.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

bool FGasXSchemaParser::LoadSchemaFromJson(const FString& JsonFilePath, FGasXAttributeSetSchema& OutSchema, FString& OutError)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
	{
		OutError = FString::Printf(TEXT("Failed to read file: %s"), *JsonFilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OutError = TEXT("Failed to parse JSON");
		return false;
	}

	// Parse schema fields
	OutSchema.AttributeSetClassName = JsonObject->GetStringField(TEXT("AttributeSetClassName"));
	OutSchema.TargetModule = JsonObject->GetStringField(TEXT("TargetModule"));
	OutSchema.TargetDirectory = JsonObject->GetStringField(TEXT("TargetDirectory"));
	OutSchema.Description = JsonObject->HasField(TEXT("Description")) ? JsonObject->GetStringField(TEXT("Description")) : TEXT("");
	OutSchema.bGenerateInitGameplayEffect = JsonObject->HasField(TEXT("bGenerateInitGameplayEffect")) ? JsonObject->GetBoolField(TEXT("bGenerateInitGameplayEffect")) : true;
	OutSchema.bGenerateMetadataTable = JsonObject->HasField(TEXT("bGenerateMetadataTable")) ? JsonObject->GetBoolField(TEXT("bGenerateMetadataTable")) : true;

	// Parse attributes array
	const TArray<TSharedPtr<FJsonValue>>* AttributesArray;
	if (!JsonObject->TryGetArrayField(TEXT("Attributes"), AttributesArray))
	{
		OutError = TEXT("Missing or invalid 'Attributes' array in JSON");
		return false;
	}

	for (const TSharedPtr<FJsonValue>& AttrValue : *AttributesArray)
	{
		if (!AttrValue.IsValid() || AttrValue->Type != EJson::Object)
		{
			continue;
		}

		FGasXAttributeDefinition AttrDef;
		if (ParseAttributeDefinition(AttrValue->AsObject(), AttrDef))
		{
			OutSchema.Attributes.Add(AttrDef);
		}
	}

	if (OutSchema.Attributes.Num() == 0)
	{
		OutError = TEXT("No valid attributes parsed from JSON");
		return false;
	}

	return true;
}

bool FGasXSchemaParser::ParseAttributeDefinition(const TSharedPtr<FJsonObject>& JsonObj, FGasXAttributeDefinition& OutAttr)
{
	if (!JsonObj.IsValid())
	{
		return false;
	}

	OutAttr.AttributeName = JsonObj->GetStringField(TEXT("AttributeName"));
	OutAttr.AttributeType = JsonObj->HasField(TEXT("AttributeType")) ? JsonObj->GetStringField(TEXT("AttributeType")) : TEXT("float");
	OutAttr.DefaultValue = JsonObj->HasField(TEXT("DefaultValue")) ? JsonObj->GetNumberField(TEXT("DefaultValue")) : 0.0;
	OutAttr.MinValue = JsonObj->HasField(TEXT("MinValue")) ? JsonObj->GetNumberField(TEXT("MinValue")) : 0.0;
	OutAttr.MaxValue = JsonObj->HasField(TEXT("MaxValue")) ? JsonObj->GetNumberField(TEXT("MaxValue")) : 100.0;
	OutAttr.bReplicates = JsonObj->HasField(TEXT("bReplicates")) ? JsonObj->GetBoolField(TEXT("bReplicates")) : true;
	OutAttr.bRepNotify = JsonObj->HasField(TEXT("bRepNotify")) ? JsonObj->GetBoolField(TEXT("bRepNotify")) : true;
	OutAttr.Description = JsonObj->HasField(TEXT("Description")) ? JsonObj->GetStringField(TEXT("Description")) : TEXT("");

	return !OutAttr.AttributeName.IsEmpty();
}
