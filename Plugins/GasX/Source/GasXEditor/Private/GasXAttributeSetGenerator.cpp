// Copyright Epic Games, Inc.

#include "GasXAttributeSetGenerator.h"
#include "Logging/LogMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/DataTableFactory.h"
#include "Factories/BlueprintFactory.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "GasXAttributeMetadata.h"
#include "UObject/SavePackage.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogGasXAttributeSetGenerator, Log, All);

bool FGasXAttributeSetGenerator::GenerateAttributeSet(
	const FGasXAttributeSetSchema &Schema,
	const FString &OutputHeaderPath,
	const FString &OutputSourcePath)
{
	FString ValidationError;
	if (!ValidateSchema(Schema, ValidationError))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Schema validation failed: %s"), *ValidationError);
		return false;
	}

	if (!EnsureOutputDirectory(FPaths::GetPath(OutputHeaderPath)))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to create output directory for header: %s"), *OutputHeaderPath);
		return false;
	}

	if (!EnsureOutputDirectory(FPaths::GetPath(OutputSourcePath)))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to create output directory for source: %s"), *OutputSourcePath);
		return false;
	}

	FString HeaderContent = GenerateHeaderContent(Schema);
	FString SourceContent = GenerateSourceContent(Schema);

	// WHY: Merge with existing files to preserve custom code outside guarded regions
	FString FinalHeaderContent = MergeWithExistingFile(OutputHeaderPath, HeaderContent);
	FString FinalSourceContent = MergeWithExistingFile(OutputSourcePath, SourceContent);

	if (!FFileHelper::SaveStringToFile(FinalHeaderContent, *OutputHeaderPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to write header file: %s"), *OutputHeaderPath);
		return false;
	}

	if (!FFileHelper::SaveStringToFile(FinalSourceContent, *OutputSourcePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to write source file: %s"), *OutputSourcePath);
		return false;
	}

	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("Successfully generated AttributeSet: %s"), *Schema.AttributeSetClassName);
	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("  Header: %s"), *OutputHeaderPath);
	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("  Source: %s"), *OutputSourcePath);

	// WHY: Optionally generate DataTable and Init GameplayEffect assets based on schema flags
	bool bAllSucceeded = true;

	if (Schema.bGenerateMetadataTable)
	{
		// WHY: Construct asset path from schema: /Game/Generated/Attributes/[ClassName]Metadata
		FString MetadataTablePath = FString::Printf(TEXT("/Game/Generated/Attributes/%sMetadata"), *Schema.AttributeSetClassName);

		if (!GenerateMetadataTable(Schema, MetadataTablePath))
		{
			UE_LOG(LogGasXAttributeSetGenerator, Warning, TEXT("Failed to generate metadata DataTable for %s"), *Schema.AttributeSetClassName);
			bAllSucceeded = false;
		}
	}

	if (Schema.bGenerateInitGameplayEffect)
	{
		// WHY: Construct asset path from schema: /Game/Generated/Attributes/GE_Init[ClassName]
		FString InitGEPath = FString::Printf(TEXT("/Game/Generated/Attributes/GE_Init%s"), *Schema.AttributeSetClassName);

		if (!GenerateInitGameplayEffect(Schema, InitGEPath))
		{
			UE_LOG(LogGasXAttributeSetGenerator, Warning, TEXT("Failed to generate Init GameplayEffect for %s"), *Schema.AttributeSetClassName);
			bAllSucceeded = false;
		}
	}

	return bAllSucceeded;
}

bool FGasXAttributeSetGenerator::ValidateSchema(const FGasXAttributeSetSchema &Schema, FString &OutError) const
{
	if (Schema.AttributeSetClassName.IsEmpty())
	{
		OutError = TEXT("AttributeSetClassName cannot be empty");
		return false;
	}

	if (!IsValidIdentifier(Schema.AttributeSetClassName))
	{
		OutError = FString::Printf(TEXT("AttributeSetClassName '%s' is not a valid C++ identifier"), *Schema.AttributeSetClassName);
		return false;
	}

	if (Schema.Attributes.Num() == 0)
	{
		OutError = TEXT("Schema must contain at least one attribute");
		return false;
	}

	TSet<FString> SeenNames;
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		if (Attr.AttributeName.IsEmpty())
		{
			OutError = TEXT("Attribute name cannot be empty");
			return false;
		}

		FString LowerName = Attr.AttributeName.ToLower();
		if (SeenNames.Contains(LowerName))
		{
			OutError = FString::Printf(TEXT("Duplicate attribute name (case-insensitive): %s"), *Attr.AttributeName);
			return false;
		}
		SeenNames.Add(LowerName);

		if (!IsValidIdentifier(Attr.AttributeName))
		{
			OutError = FString::Printf(TEXT("Attribute name '%s' is not a valid C++ identifier"), *Attr.AttributeName);
			return false;
		}

		if (IsReservedKeyword(Attr.AttributeName))
		{
			OutError = FString::Printf(TEXT("Attribute name '%s' is a reserved C++ keyword"), *Attr.AttributeName);
			return false;
		}

		if (Attr.AttributeType != TEXT("float") && Attr.AttributeType != TEXT("int32"))
		{
			OutError = FString::Printf(TEXT("Attribute type '%s' not supported in MVP (use 'float' or 'int32')"), *Attr.AttributeType);
			return false;
		}
	}

	return true;
}

FString FGasXAttributeSetGenerator::GenerateHeaderContent(const FGasXAttributeSetSchema &Schema) const
{
	FString Header = TEXT("// Copyright Epic Games, Inc.\n");
	Header += TEXT("// AUTO-GENERATED by GasXAttributeSetGenerator - DO NOT EDIT MANUALLY\n\n");
	Header += TEXT("#pragma once\n\n");
	Header += TEXT("#include \"CoreMinimal.h\"\n");
	Header += TEXT("#include \"AttributeSet.h\"\n");
	Header += TEXT("#include \"AbilitySystemComponent.h\"\n");
	Header += FString::Printf(TEXT("#include \"%s.generated.h\"\n\n"), *Schema.AttributeSetClassName);

	Header += TEXT("/**\n");
	Header += FString::Printf(TEXT(" * Generated AttributeSet: %s\n"), *Schema.AttributeSetClassName);
	if (!Schema.Description.IsEmpty())
	{
		Header += FString::Printf(TEXT(" * %s\n"), *Schema.Description);
	}
	Header += TEXT(" */\n");
	Header += TEXT("UCLASS()\n");
	Header += FString::Printf(TEXT("class %s_API U%s : public UAttributeSet\n"), *Schema.TargetModule.ToUpper(), *Schema.AttributeSetClassName);
	Header += TEXT("{\n");
	Header += TEXT("\tGENERATED_BODY()\n\n");
	Header += TEXT("public:\n");
	Header += FString::Printf(TEXT("\tU%s();\n\n"), *Schema.AttributeSetClassName);

	// Generate attribute properties
	Header += TEXT("\t//GEN-BEGIN: Attribute Properties\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		Header += GenerateAttributeProperty(Attr);
	}
	Header += TEXT("\t//GEN-END: Attribute Properties\n\n");

	// Generate accessors
	Header += TEXT("\t//GEN-BEGIN: Attribute Accessors\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		Header += GenerateAccessors(Attr, Schema.AttributeSetClassName);
	}
	Header += TEXT("\t//GEN-END: Attribute Accessors\n\n");

	// Generate OnRep declarations
	Header += TEXT("\t//GEN-BEGIN: OnRep Functions\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		if (Attr.bReplicates && Attr.bRepNotify)
		{
			Header += GenerateOnRepDeclaration(Attr);
		}
	}
	Header += TEXT("\t//GEN-END: OnRep Functions\n\n");

	Header += TEXT("\tvirtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;\n");
	Header += TEXT("};\n");

	return Header;
}

FString FGasXAttributeSetGenerator::GenerateSourceContent(const FGasXAttributeSetSchema &Schema) const
{
	FString Source = TEXT("// Copyright Epic Games, Inc.\n");
	Source += TEXT("// AUTO-GENERATED by GasXAttributeSetGenerator - DO NOT EDIT MANUALLY\n\n");
	Source += FString::Printf(TEXT("#include \"Attributes/%s.h\"\n"), *Schema.AttributeSetClassName);
	Source += TEXT("#include \"Net/UnrealNetwork.h\"\n\n");

	// Constructor
	Source += FString::Printf(TEXT("U%s::U%s()\n"), *Schema.AttributeSetClassName, *Schema.AttributeSetClassName);
	Source += TEXT("{\n");
	Source += TEXT("\t//GEN-BEGIN: Constructor Initialization\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		Source += FString::Printf(TEXT("\t%s.SetBaseValue(%.2ff);\n"), *Attr.AttributeName, Attr.DefaultValue);
		Source += FString::Printf(TEXT("\t%s.SetCurrentValue(%.2ff);\n"), *Attr.AttributeName, Attr.DefaultValue);
	}
	Source += TEXT("\t//GEN-END: Constructor Initialization\n");
	Source += TEXT("}\n\n");

	// OnRep implementations
	Source += TEXT("//GEN-BEGIN: OnRep Implementations\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		if (Attr.bReplicates && Attr.bRepNotify)
		{
			Source += GenerateOnRepImplementation(Attr, Schema.AttributeSetClassName);
		}
	}
	Source += TEXT("//GEN-END: OnRep Implementations\n\n");

	// Replication setup
	Source += GenerateReplicationSetup(Schema);

	return Source;
}

FString FGasXAttributeSetGenerator::GenerateAttributeProperty(const FGasXAttributeDefinition &Attribute) const
{
	FString Property;
	if (!Attribute.Description.IsEmpty())
	{
		Property += FString::Printf(TEXT("\t/** %s */\n"), *Attribute.Description);
	}

	Property += TEXT("\tUPROPERTY(BlueprintReadOnly, Category=\"Attributes\"");
	if (Attribute.bReplicates && Attribute.bRepNotify)
	{
		Property += FString::Printf(TEXT(", ReplicatedUsing=OnRep_%s"), *Attribute.AttributeName);
	}
	Property += TEXT(")\n");
	Property += FString::Printf(TEXT("\tFGameplayAttributeData %s;\n\n"), *Attribute.AttributeName);

	return Property;
}

FString FGasXAttributeSetGenerator::GenerateOnRepDeclaration(const FGasXAttributeDefinition &Attribute) const
{
	FString Decl = TEXT("\tUFUNCTION()\n");
	Decl += FString::Printf(TEXT("\tvirtual void OnRep_%s(const FGameplayAttributeData& OldValue);\n\n"), *Attribute.AttributeName);
	return Decl;
}

FString FGasXAttributeSetGenerator::GenerateAccessors(const FGasXAttributeDefinition &Attribute, const FString &ClassName) const
{
	FString Accessors;
	Accessors += FString::Printf(TEXT("\tGAMEPLAYATTRIBUTE_PROPERTY_GETTER(U%s, %s)\n"), *ClassName, *Attribute.AttributeName);
	Accessors += FString::Printf(TEXT("\tGAMEPLAYATTRIBUTE_VALUE_GETTER(%s)\n"), *Attribute.AttributeName);
	Accessors += FString::Printf(TEXT("\tGAMEPLAYATTRIBUTE_VALUE_SETTER(%s)\n"), *Attribute.AttributeName);
	Accessors += FString::Printf(TEXT("\tGAMEPLAYATTRIBUTE_VALUE_INITTER(%s)\n\n"), *Attribute.AttributeName);
	return Accessors;
}

FString FGasXAttributeSetGenerator::GenerateOnRepImplementation(const FGasXAttributeDefinition &Attribute, const FString &ClassName) const
{
	FString Impl;
	Impl += FString::Printf(TEXT("void U%s::OnRep_%s(const FGameplayAttributeData& OldValue)\n"), *ClassName, *Attribute.AttributeName);
	Impl += TEXT("{\n");
	Impl += FString::Printf(TEXT("\tGAMEPLAYATTRIBUTE_REPNOTIFY(U%s, %s, OldValue);\n"), *ClassName, *Attribute.AttributeName);
	Impl += TEXT("}\n\n");
	return Impl;
}

FString FGasXAttributeSetGenerator::GenerateReplicationSetup(const FGasXAttributeSetSchema &Schema) const
{
	FString Repl;
	Repl += FString::Printf(TEXT("void U%s::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const\n"), *Schema.AttributeSetClassName);
	Repl += TEXT("{\n");
	Repl += TEXT("\tSuper::GetLifetimeReplicatedProps(OutLifetimeProps);\n\n");
	Repl += TEXT("\t//GEN-BEGIN: Replication Setup\n");
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		if (Attr.bReplicates)
		{
			Repl += FString::Printf(TEXT("\tDOREPLIFETIME_CONDITION_NOTIFY(U%s, %s, COND_None, REPNOTIFY_Always);\n"),
									*Schema.AttributeSetClassName, *Attr.AttributeName);
		}
	}
	Repl += TEXT("\t//GEN-END: Replication Setup\n");
	Repl += TEXT("}\n");
	return Repl;
}

bool FGasXAttributeSetGenerator::EnsureOutputDirectory(const FString &DirectoryPath) const
{
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*DirectoryPath))
	{
		return PlatformFile.CreateDirectoryTree(*DirectoryPath);
	}
	return true;
}

bool FGasXAttributeSetGenerator::IsReservedKeyword(const FString &Name) const
{
	static const TSet<FString> ReservedKeywords = {
		TEXT("alignas"), TEXT("alignof"), TEXT("and"), TEXT("and_eq"), TEXT("asm"), TEXT("auto"),
		TEXT("bitand"), TEXT("bitor"), TEXT("bool"), TEXT("break"), TEXT("case"), TEXT("catch"),
		TEXT("char"), TEXT("char16_t"), TEXT("char32_t"), TEXT("class"), TEXT("compl"), TEXT("const"),
		TEXT("constexpr"), TEXT("const_cast"), TEXT("continue"), TEXT("decltype"), TEXT("default"),
		TEXT("delete"), TEXT("do"), TEXT("double"), TEXT("dynamic_cast"), TEXT("else"), TEXT("enum"),
		TEXT("explicit"), TEXT("export"), TEXT("extern"), TEXT("false"), TEXT("float"), TEXT("for"),
		TEXT("friend"), TEXT("goto"), TEXT("if"), TEXT("inline"), TEXT("int"), TEXT("long"),
		TEXT("mutable"), TEXT("namespace"), TEXT("new"), TEXT("noexcept"), TEXT("not"), TEXT("not_eq"),
		TEXT("nullptr"), TEXT("operator"), TEXT("or"), TEXT("or_eq"), TEXT("private"), TEXT("protected"),
		TEXT("public"), TEXT("register"), TEXT("reinterpret_cast"), TEXT("return"), TEXT("short"),
		TEXT("signed"), TEXT("sizeof"), TEXT("static"), TEXT("static_assert"), TEXT("static_cast"),
		TEXT("struct"), TEXT("switch"), TEXT("template"), TEXT("this"), TEXT("thread_local"),
		TEXT("throw"), TEXT("true"), TEXT("try"), TEXT("typedef"), TEXT("typeid"), TEXT("typename"),
		TEXT("union"), TEXT("unsigned"), TEXT("using"), TEXT("virtual"), TEXT("void"), TEXT("volatile"),
		TEXT("wchar_t"), TEXT("while"), TEXT("xor"), TEXT("xor_eq")};

	return ReservedKeywords.Contains(Name.ToLower());
}

bool FGasXAttributeSetGenerator::IsValidIdentifier(const FString &Name) const
{
	if (Name.IsEmpty())
	{
		return false;
	}

	// First character must be letter or underscore
	TCHAR FirstChar = Name[0];
	if (!FChar::IsAlpha(FirstChar) && FirstChar != TEXT('_'))
	{
		return false;
	}

	// Remaining characters must be alphanumeric or underscore
	for (int32 i = 1; i < Name.Len(); ++i)
	{
		TCHAR Ch = Name[i];
		if (!FChar::IsAlnum(Ch) && Ch != TEXT('_'))
		{
			return false;
		}
	}

	return true;
}

FString FGasXAttributeSetGenerator::MergeWithExistingFile(const FString &FilePath, const FString &NewGeneratedContent) const
{
	// WHY: If file doesn't exist, this is first generation - use new content as-is
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("Creating new file: %s"), *FilePath);
		return NewGeneratedContent;
	}

	FString ExistingContent;
	if (!FFileHelper::LoadFileToString(ExistingContent, *FilePath))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Warning, TEXT("Could not read existing file: %s. Using new content."), *FilePath);
		return NewGeneratedContent;
	}

	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("Merging with existing file: %s"), *FilePath);

	// Start from the existing file so custom code outside guarded regions stays untouched
	return ReplaceGuardedRegions(ExistingContent, NewGeneratedContent);
}

FString FGasXAttributeSetGenerator::ReplaceGuardedRegions(const FString &ExistingContent, const FString &NewContent) const
{
	// WHY: Preserve developer code outside guarded regions by editing the existing file in-place.
	FString Result = ExistingContent;
	struct FGuardBlock
	{
		FString Name;
		FString BlockText;
	};

	TArray<FGuardBlock> NewBlocks;
	const FString BeginSignature = TEXT("//GEN-BEGIN:");
	int32 SearchPosition = 0;

	while (SearchPosition < NewContent.Len())
	{
		const int32 BeginIndex = NewContent.Find(BeginSignature, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPosition);
		if (BeginIndex == INDEX_NONE)
		{
			break;
		}

		const int32 BeginLineEnd = NewContent.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, BeginIndex);
		const int32 SafeBeginLineEnd = BeginLineEnd == INDEX_NONE ? NewContent.Len() : BeginLineEnd + 1;

		FString RegionName = NewContent.Mid(BeginIndex + BeginSignature.Len(), SafeBeginLineEnd - (BeginIndex + BeginSignature.Len()));
		RegionName = RegionName.TrimStartAndEnd();

		const FString EndMarker = FString::Printf(TEXT("//GEN-END: %s"), *RegionName);
		const int32 EndIndex = NewContent.Find(EndMarker, ESearchCase::CaseSensitive, ESearchDir::FromStart, SafeBeginLineEnd);
		if (EndIndex == INDEX_NONE)
		{
			break;
		}

		const int32 EndLineEnd = NewContent.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, EndIndex);
		const int32 SafeEndLineEnd = EndLineEnd == INDEX_NONE ? NewContent.Len() : EndLineEnd + 1;

		NewBlocks.Add({RegionName, NewContent.Mid(BeginIndex, SafeEndLineEnd - BeginIndex)});
		SearchPosition = SafeEndLineEnd;
	}

	if (NewBlocks.Num() == 0)
	{
		return NewContent;
	}

	bool bModified = false;

	for (const FGuardBlock &Block : NewBlocks)
	{
		const FString BeginNeedle = FString::Printf(TEXT("GEN-BEGIN: %s"), *Block.Name);
		const FString EndNeedle = FString::Printf(TEXT("GEN-END: %s"), *Block.Name);

		const int32 BeginNeedleIndex = Result.Find(BeginNeedle, ESearchCase::CaseSensitive);
		if (BeginNeedleIndex == INDEX_NONE)
		{
			continue;
		}

		const int32 BlockStartLine = [&Result](int32 Index)
		{
			const int32 PrevNewline = Result.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, Index);
			return PrevNewline == INDEX_NONE ? 0 : PrevNewline + 1;
		}(BeginNeedleIndex);

		const int32 EndNeedleIndex = Result.Find(EndNeedle, ESearchCase::CaseSensitive, ESearchDir::FromStart, BeginNeedleIndex);
		if (EndNeedleIndex == INDEX_NONE)
		{
			continue;
		}

		const int32 BlockEndLine = [&Result](int32 Index)
		{
			int32 NextNewline = Result.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Index);
			return NextNewline == INDEX_NONE ? Result.Len() : NextNewline + 1;
		}(EndNeedleIndex);

		Result = Result.Left(BlockStartLine) + Block.BlockText + Result.Mid(BlockEndLine);
		bModified = true;
	}

	return Result;
}

bool FGasXAttributeSetGenerator::GenerateMetadataTable(const FGasXAttributeSetSchema &Schema, const FString &OutputAssetPath)
{
	// WHY: Create a UDataTable with FGasXAttributeMetadataRow structure to hold designer-editable attribute values
	// NOTE: This must run in editor context with AssetTools available

#if WITH_EDITOR
	// WHY: AssetTools provides the CreateAsset() API for programmatic asset generation
	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// WHY: DataTableFactory is required to create UDataTable instances
	UDataTableFactory *Factory = NewObject<UDataTableFactory>();
	Factory->Struct = FGasXAttributeMetadataRow::StaticStruct();

	// WHY: Parse the output path into package and asset names
	// Example: "/Game/Generated/Attributes/PlayerCoreMetadata" → Package="/Game/Generated/Attributes", Asset="PlayerCoreMetadata"
	FString PackagePath, AssetName;
	OutputAssetPath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	if (PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Invalid OutputAssetPath format: %s (expected /Game/Path/AssetName)"), *OutputAssetPath);
		return false;
	}

	// WHY: Create the DataTable asset in the specified package
	UDataTable *DataTable = Cast<UDataTable>(AssetTools.CreateAsset(
		AssetName,
		PackagePath,
		UDataTable::StaticClass(),
		Factory));

	if (!DataTable)
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to create DataTable asset: %s"), *OutputAssetPath);
		return false;
	}

	// WHY: Populate the DataTable with rows for each attribute from the schema
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
	const FName RowName = FName(*Attr.AttributeName);
	FGasXAttributeMetadataRow RowData;
	RowData.BaseValue = Attr.DefaultValue;
	RowData.MinValue = Attr.MinValue;
	RowData.MaxValue = Attr.MaxValue;
	RowData.Description = Attr.Description.IsEmpty()
							  ? FString::Printf(TEXT("Metadata for %s attribute"), *Attr.AttributeName)
							  : Attr.Description;

	if (FGasXAttributeMetadataRow *ExistingRow = DataTable->FindRow<FGasXAttributeMetadataRow>(RowName, TEXT("GasXGenerator"), /*bWarnIfRowMissing*/ false))
	{
		*ExistingRow = RowData;
	}
	else
	{
		DataTable->AddRow(RowName, RowData);
	}
	}

	// WHY: Mark the asset dirty so it will be saved with the project
	DataTable->MarkPackageDirty();

	// WHY: Trigger a save to disk
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath / AssetName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	SaveArgs.Error = GError;

	if (!UPackage::SavePackage(DataTable->GetOutermost(), DataTable, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Warning, TEXT("Failed to save DataTable package: %s"), *PackageFileName);
		// WHY: Don't return false - asset is still created in memory, just not saved yet
	}

	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("Successfully generated DataTable: %s (%d rows)"), *OutputAssetPath, Schema.Attributes.Num());
	return true;

#else
	UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("GenerateMetadataTable requires WITH_EDITOR - cannot run in packaged build"));
	return false;
#endif
}

bool FGasXAttributeSetGenerator::GenerateInitGameplayEffect(const FGasXAttributeSetSchema &Schema, const FString &OutputAssetPath)
{
	// WHY: Create a UGameplayEffect asset with Instant modifiers to initialize attributes from schema defaults
	// NOTE: This must run in editor context with AssetTools available

#if WITH_EDITOR
	// WHY: AssetTools provides the CreateAsset() API for programmatic asset generation
	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// WHY: Parse the output path into package and asset names
	FString PackagePath, AssetName;
	OutputAssetPath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	if (PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Invalid OutputAssetPath format: %s (expected /Game/Path/AssetName)"), *OutputAssetPath);
		return false;
	}

	// WHY: GameplayEffect assets are created as blueprints derived from UGameplayEffect
	UBlueprintFactory *Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = UGameplayEffect::StaticClass();

	// WHY: Create the GameplayEffect blueprint asset
	UBlueprint *EffectBlueprint = Cast<UBlueprint>(AssetTools.CreateAsset(
		AssetName,
		PackagePath,
		UBlueprint::StaticClass(),
		Factory));

	if (!EffectBlueprint)
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to create GameplayEffect blueprint: %s"), *OutputAssetPath);
		return false;
	}

	// WHY: Access the CDO (Class Default Object) to configure the GameplayEffect properties
	UGameplayEffect *EffectCDO = Cast<UGameplayEffect>(EffectBlueprint->GeneratedClass->GetDefaultObject());
	if (!EffectCDO)
	{
		UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("Failed to get GameplayEffect CDO from blueprint: %s"), *OutputAssetPath);
		return false;
	}

	// WHY: Init effects must be Instant duration to apply immediately
	EffectCDO->DurationPolicy = EGameplayEffectDurationType::Instant;

	// WHY: Add modifiers for each attribute in the schema
	// NOTE: Each modifier sets the attribute's base value to the schema default
	// NOTE: Attribute binding requires compiled reflection data; warn if generation happens before compile.
	for (const FGasXAttributeDefinition &Attr : Schema.Attributes)
	{
		FGameplayModifierInfo Modifier;

		UE_LOG(LogGasXAttributeSetGenerator, Verbose, TEXT("Init GE modifier for %s requires manual attribute binding after compile."), *Attr.AttributeName);

		// WHY: Override operation replaces the attribute's current value
		Modifier.ModifierOp = EGameplayModOp::Override;

		// WHY: Use a constant magnitude equal to the schema default
		FScalableFloat ScalableValue;
		ScalableValue.Value = Attr.DefaultValue;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(ScalableValue);

		EffectCDO->Modifiers.Add(Modifier);
	}

	// WHY: Mark the blueprint dirty so it will be saved with the project
	EffectBlueprint->MarkPackageDirty();

	// WHY: Compile the blueprint to ensure the changes are applied
	FKismetEditorUtilities::CompileBlueprint(EffectBlueprint, EBlueprintCompileOptions::None);

	// WHY: Trigger a save to disk
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath / AssetName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	SaveArgs.Error = GError;

	if (!UPackage::SavePackage(EffectBlueprint->GetOutermost(), EffectBlueprint, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogGasXAttributeSetGenerator, Warning, TEXT("Failed to save GameplayEffect package: %s"), *PackageFileName);
		// WHY: Don't return false - asset is still created in memory, just not saved yet
	}

	UE_LOG(LogGasXAttributeSetGenerator, Log, TEXT("Successfully generated Init GameplayEffect: %s (%d modifiers)"), *OutputAssetPath, Schema.Attributes.Num());
	return true;

#else
	UE_LOG(LogGasXAttributeSetGenerator, Error, TEXT("GenerateInitGameplayEffect requires WITH_EDITOR - cannot run in packaged build"));
	return false;
#endif
}
