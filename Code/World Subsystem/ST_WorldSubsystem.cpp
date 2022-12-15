// Copyright (c) James Baxter. All Rights Reserved.

#include "ST_WorldSubsystem.h"

// Engine
#include "Engine/World.h"
#include "GameMapsSettings.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Engine/NetDriver.h"
#endif

/////////////////////////
///// Tick Function /////
/////////////////////////

void FST_WorldSubsystemTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	check(Target);
	Target->TickSubsystem(DeltaTime);
}

FString FST_WorldSubsystemTickFunction::DiagnosticMessage()
{
	return TEXT("HT_WorldManager::Tick()");
}

FName FST_WorldSubsystemTickFunction::DiagnosticContext(bool bDetailed)
{
	check(Target);
	return Target->GetClass()->GetFName();
}

///////////////////////
///// Constructor /////
///////////////////////

UST_WorldSubsystemBase::UST_WorldSubsystemBase()
	: UWorldSubsystem()
{
	SubsystemTickFunction.bCanEverTick = false;
	SubsystemTickFunction.bStartWithTickEnabled = false;
	SubsystemTickFunction.bAllowTickOnDedicatedServer = true;
	SubsystemTickFunction.TickGroup = ETickingGroup::TG_PrePhysics;

	// Skip 'Entry' and 'MainMenu' levels by default..
	LevelBlocklist.Add("UM_Entry");
	LevelBlocklist.Add("UM_MainMenu");

	bEnableInUntitledLevel = false;
	bEnableInTransitionLevel = false;

	// Initialise in all network domains by default
	InitialisationNetModeMask |= (uint8)1 << static_cast<uint8>(ENetMode::NM_Standalone);
	InitialisationNetModeMask |= (uint8)1 << static_cast<uint8>(ENetMode::NM_DedicatedServer);
	InitialisationNetModeMask |= (uint8)1 << static_cast<uint8>(ENetMode::NM_ListenServer);
	InitialisationNetModeMask |= (uint8)1 << static_cast<uint8>(ENetMode::NM_Client);
}

/////////////////////
///// Lifecycle /////
/////////////////////

bool UST_WorldSubsystemBase::ShouldCreateSubsystem(UObject* InOuter) const
{
	if (Super::ShouldCreateSubsystem(InOuter))
	{
		const UWorld* lWorld = Cast<UWorld>(InOuter);
		if (lWorld && lWorld->IsGameWorld())
		{
#if WITH_EDITOR
			// PIE creates temporary worlds for Net Clients which should really never initialize subsystems.
			if (lWorld->IsPlayInEditor())
			{
				const UPackage* OuterPackage = CastChecked<UPackage>(lWorld->GetOuter());
				if (!OuterPackage || OuterPackage->GetPIEInstanceID() == INDEX_NONE)
				{
					return false;
				}
			}
#endif
			if (CheckNetMode(lWorld) && CheckLevelName(lWorld))
			{
				return true;
			}
		}
	}

	return false;
}

/////////////////////////
///// World Manager /////
/////////////////////////

void UST_WorldSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWorld* lWorld = GetWorld();
	check(lWorld && lWorld->IsGameWorld() && lWorld->PersistentLevel != nullptr);

	// Can't actually do safe initialisation until much later...
	PostInitWorldDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &URLHWorldSubsystemBase::PostInitWorldInternal);

#if WITH_EDITOR
	// Editor environment is added fun
	PostInitWorldPIEDelegateHandle = FEditorDelegates::PostPIEStarted.AddUObject(this, &URLHWorldSubsystemBase::PostInitWorldPIEInternal);
#endif
}

void URLHWorldSubsystemBase::Deinitialize()
{
	SubsystemTickFunction.UnRegisterTickFunction();
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostInitWorldDelegateHandle);
#if WITH_EDITOR
	FEditorDelegates::PostPIEStarted.Remove(PostInitWorldPIEDelegateHandle);
#endif

	Super::Deinitialize();
}

bool UST_WorldSubsystemBase::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UST_WorldSubsystemBase::PostInitWorldInternal(UWorld* NewWorld)
{
	const UWorld* lWorld = GetWorld();
	if (lWorld && lWorld == NewWorld && !bHasPostWorldInitialized)
	{
		bHasPostWorldInitialized = true;

		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostInitWorldDelegateHandle);
#if WITH_EDITOR
		FEditorDelegates::PostPIEStarted.Remove(PostInitWorldPIEDelegateHandle);
#endif

		// Handle Tick Registration now.
		if (!SubsystemTickFunction.IsTickFunctionRegistered() && SubsystemTickFunction.bCanEverTick)
		{
			SubsystemTickFunction.Target = this;
			SubsystemTickFunction.SetTickFunctionEnable(SubsystemTickFunction.bStartWithTickEnabled || SubsystemTickFunction.IsTickFunctionEnabled());
			SubsystemTickFunction.RegisterTickFunction(lWorld->PersistentLevel);
		}

		OnWorldInitialized();
	}
}

#if WITH_EDITOR
void UST_WorldSubsystemBase::PostInitWorldPIEInternal(bool bSimulating)
{
	PostInitWorldInternal(GetWorld());
}
#endif

/////////////////////////////////
///// Initialization Checks /////
/////////////////////////////////

bool UST_WorldSubsystemBase::CheckNetMode(const UWorld* lWorld) const
{
	ENetMode MyNetMode = ENetMode::NM_MAX;
	if (GetSafeNetMode(MyNetMode, lWorld))
	{
		const uint8 NetModeMask = (uint8)1 << (uint8)MyNetMode;
		return (InitialisationNetModeMask & NetModeMask) != 0;
	}

	return false;
}

bool UST_WorldSubsystemBase::CheckLevelName(const UWorld* lWorld) const
{
	checkSlow(lWorld);

	// Check Level Names
	FString LevelName = lWorld->GetMapName();
	LevelName.RemoveFromStart(lWorld->StreamingLevelsPrefix);

	// Optionally Block in the 'Untitled' Level
	// These are special levels the engine sometimes creates for intermediate UWorlds.
	if (!bEnableInUntitledLevel && LevelName.Contains(TEXT("Untitled")))
	{
		return false;
	}

	// Optionally Block in the 'Transition' Level
	// Read the Transition Level from project settings.
	const UGameMapsSettings* MapSettings = GetDefault<UGameMapsSettings>();
	check(MapSettings);

	if (!bEnableInTransitionLevel && !MapSettings->TransitionMap.IsNull() && MapSettings->TransitionMap.GetAssetName() == LevelName)
	{
		return false;
	}

	if (LevelAllowlist.Num() != 0 && !LevelAllowlist.Contains(LevelName))
	{
		return false;
	}

	if (LevelBlocklist.Contains(LevelName))
	{
		return false;
	}

	return true;
}

/////////////////////
///// Utilities /////
/////////////////////

bool UST_WorldSubsystemBase::GetSafeNetMode(ENetMode& OutMode, const UWorld* OverrideWorld /*= nullptr*/) const
{
	const UWorld* lWorld = OverrideWorld ? OverrideWorld : GetWorld();
	check(lWorld);

	OutMode = lWorld->GetNetMode();

#if WITH_EDITOR
	// PIE creates some obscure UWorld lifetimes that make NetMode access more difficult.
	// See https://udn.unrealengine.com/s/question/0D54z00007DW1CQCA1/why-are-uworldsubsystems-initialized-before-uworld-is-initialized
	if (lWorld->IsPlayInEditor())
	{
		UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GEngine);
		const UPackage* Package = Cast<UPackage>(lWorld->GetOuter());
		if (!Package || Package->GetPIEInstanceID() == INDEX_NONE)
		{
			// This is a Transient UPackage used for PIE Client worlds.
			OutMode = ENetMode::NM_MAX;
			return false;
		}
		else
		{
			// We can get the UGameInstance/UNetDriver from here, *before* the UWorld properties are set
			const FWorldContext* PIEWC = EditorEngine->GetWorldContextFromPIEInstance(Package->GetPIEInstanceID());
			if (ensureAlways(PIEWC))
			{
				if (!PIEWC->PendingNetGame)
				{
					OutMode = PIEWC->RunAsDedicated ? ENetMode::NM_DedicatedServer : ENetMode::NM_Standalone;
				}
				else if (ensureAlways(PIEWC->PendingNetGame->NetDriver))
				{
					OutMode = PIEWC->PendingNetGame->NetDriver->GetNetMode();
				}
			}
		}
	}
#endif

	return true;
}