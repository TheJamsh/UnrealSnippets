// Copyright (c) James Baxter. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "ST_WorldSubsystem.generated.h"

// Declarations
class UST_WorldSubsystem;

/*
* World Subsystem Tick Function
* Gives us a tick with a selectable tick group
*/
USTRUCT()
struct FST_WorldSubsystemTickFunction : public FTickFunction
{
	GENERATED_BODY()
public:
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override final;
	virtual FString DiagnosticMessage() override final;
	virtual FName DiagnosticContext(bool bDetailed) override final;

private:
	friend UST_WorldSubsystem;
	UST_WorldSubsystem* Target;
};

template<>
struct TStructOpsTypeTraits<FST_WorldSubsystemTickFunction> : public TStructOpsTypeTraitsBase2<FST_WorldSubsystemTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

/*
* ST WorldSubsystem
* Improves over the engines base world subsystem by adding a convenient level-tick function, and network checks
*/
UCLASS(Abstract, NotBlueprintType, NotBlueprintable)
class UST_WorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UST_WorldSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* InOuter) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool GetSafeNetMode(ENetMode& OutMode, const UWorld* OverrideWorld = nullptr) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	friend FST_WorldSubsystemTickFunction;

	/* Virtual stub that can be overridden in a child class to perform safer initialisation. */
	virtual void OnWorldInitialized() {}
	virtual void TickSubsystem(const float InDeltaTime) {}

	bool CheckNetMode(const UWorld* lWorld) const;
	bool CheckLevelName(const UWorld* lWorld) const;

	/* If non-empty, the subsystem will *NOT* be initialised if the level name is in this list. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialisation")
	TSet<FString> LevelBlocklist;

	/* If non-empty, the subsystem will only be initialised if the level name is in this list. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialisation")
	TSet<FString> LevelAllowlist;

	/* Don't initialise the subsystem unless the world matches the given Net Modes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialisation", meta = (Bitmask, BitmaskEnum = "ENetMode"))
	uint8 InitialisationNetModeMask;

	/* If true, enables in an "Untitled" level. These are usually special intermediate/transient levels created by the engine. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialisation", AdvancedDisplay)
	uint8 bEnableInUntitledLevel : 1;

	/* If true, enables in the Transition Level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialisation", AdvancedDisplay)
	uint8 bEnableInTransitionLevel : 1;

	/* Tick Function */
	UPROPERTY(EditDefaultsOnly, Category = "Subsystem Tick")
	FST_WorldSubsystemTickFunction SubsystemTickFunction;

private:
	FDelegateHandle PostInitWorldDelegateHandle;
	bool bHasPostWorldInitialized;

	void PostInitWorldInternal(UWorld* NewWorld);

#if WITH_EDITOR
	FDelegateHandle PostInitWorldPIEDelegateHandle;
	void PostInitWorldPIEInternal(bool bSimulating);
#endif
};