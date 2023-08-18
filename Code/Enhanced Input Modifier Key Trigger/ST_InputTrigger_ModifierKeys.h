// Copyright (C) James Baxter. All Rights Reserved.

#pragma once

#include "InputTriggers.h"
#include "UObject/ObjectSaveContext.h"
#include "ST_InputTrigger_ModifierKeys.generated.h"

USTRUCT(BlueprintType)
struct FST_InputTriggerModifiers
{
	GENERATED_BODY()
public:
	FHT_InputTriggerModifiers() = default;

	explicit FHT_InputTriggerModifiers(const bool bInShift, const bool bInCtrl, const bool bInAlt, const bool bInCmd)
		: bShift(bInShift)
		, bCtrl(bInCtrl)
		, bAlt(bInAlt)
		, bCmd(bInCmd)
	{}

	bool HasAnyModifiers() const { return bShift || bCtrl || bAlt || bCmd; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") uint8 bShift : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") uint8 bCtrl : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") uint8 bAlt : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") uint8 bCmd : 1;

	bool operator==(const FHT_InputTriggerModifiers& RHS) const { return bShift == RHS.bShift && bCtrl == RHS.bCtrl && bAlt == RHS.bAlt && bCmd == RHS.bCmd; }
	bool operator!=(const FHT_InputTriggerModifiers& RHS) const { return bShift != RHS.bShift || bCtrl != RHS.bCtrl || bAlt != RHS.bAlt || bCmd != RHS.bCmd; }

	FString ToString() const { return FString::Printf(TEXT("Shift [%i] - Ctrl [%i] - Alt [%i] - Cmd [%i]"), bShift, bCtrl, bAlt, bCmd); }
};

/*
* Allows modifier keys to modify trigger state
*/
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Modifier Keys"))
class UST_InputTriggerModifierKeys final : public UInputTrigger
{
	GENERATED_BODY()
public:
	// Constructor
	UST_InputTriggerModifierKeys(const FObjectInitializer& OI);

	// UObject Interface
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

	// UInputTrigger Interface
public:
	virtual ETriggerEventsSupported GetSupportedTriggerEvents() const override { return ETriggerEventsSupported::Instant; }

protected:
	virtual ETriggerType GetTriggerType_Implementation() const override { return ETriggerType::Implicit; }
	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;

	/** Modifiers keys required to trigger action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ShowOnlyInnerProperties))
	FHT_InputTriggerModifiers RequiredModifierKeys = {};

	/*
	* If true, modifier keys can be remapped by the user.
	* If false, modifier keys are locked to whatever is set here.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ShowOnlyInnerProperties))
	bool bModifiersAreMappable = true;

	/*
	* Parent Context/Action. Serialized so we can link to action/context key remap settings
	* #TODO: Make Transient, and restore from PostLoad()?
	*/
	UPROPERTY(VisibleDefaultsOnly, Category = "Input", meta = (DisplayThumbnail = "false")) TSoftObjectPtr<const UObject> TemplateOuter;
	UPROPERTY(VisibleDefaultsOnly, Category = "Input") int32 TemplateIndex;
};