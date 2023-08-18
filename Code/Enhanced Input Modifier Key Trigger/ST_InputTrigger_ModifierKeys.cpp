// Copyright (C) James Baxter. All Rights Reserved.

#include "ST_InputTrigger_ModifierKeys.h"

// Engine
#include "EnhancedPlayerInput.h"
#include "InputMappingContext.h"

/////////////////////////
///// Modifier Keys /////
/////////////////////////

UST_InputTriggerModifierKeys::UST_InputTriggerModifierKeys(const FObjectInitializer& OI)
	: Super(OI)
{}

void UST_InputTriggerModifierKeys::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	const UObject* Parent = GetOuter();
	if (!Parent->HasAnyFlags(RF_Transient) && bModifiersAreMappable)
	{
		if (const UInputMappingContext* OuterContext = Cast<UInputMappingContext>(GetOuter()))
		{
			TemplateOuter = OuterContext;
			TemplateIndex = INDEX_NONE;

			for (int32 Idx = 0; Idx < OuterContext->GetMappings().Num(); Idx++)
			{
				const FEnhancedActionKeyMapping& RHS = OuterContext->GetMappings()[Idx];
				const int32 TriggerIndex = RHS.Triggers.IndexOfByKey(this);
				if (TriggerIndex != INDEX_NONE)
				{
					TemplateIndex = TriggerIndex;
					break;
				}
			}

			check(TemplateIndex != INDEX_NONE);
		}
		else if (const UInputAction* OuterAction = Cast<UInputAction>(GetOuter()))
		{
			TemplateOuter = OuterAction;
			TemplateIndex = INDEX_NONE;
		}
		else
		{
			TemplateOuter = nullptr;
			TemplateIndex = INDEX_NONE;
		}
	}
	else
	{
		TemplateOuter = nullptr;
		TemplateIndex = INDEX_NONE;
	}
}

ETriggerState UST_InputTriggerModifierKeys::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	// #TODO: Need to check if chorded keys interfere..?

	// #TODO
	// Read applicable keys from player action mapping. Will need to cache to avoid constant lookups somewhere.
	const FHT_InputTriggerModifiers CurrentKeys = FHT_InputTriggerModifiers(
		PlayerInput->IsShiftPressed(),
		PlayerInput->IsCtrlPressed(),
		PlayerInput->IsAltPressed(),
		PlayerInput->IsCmdPressed());

	const bool bModifiersMatch = RequiredModifierKeys == CurrentKeys;
	return bModifiersMatch ? ETriggerState::Triggered : ETriggerState::None;
}

#if WITH_EDITOR
EDataValidationResult UST_InputTriggerModifierKeys::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	if (!bModifiersAreMappable && !RequiredModifierKeys.HasAnyModifiers())
	{
		ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Modifier Trigger has no modifiers and is not rebindable"))));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif