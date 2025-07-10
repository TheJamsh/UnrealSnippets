#include "NetworkEventSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY_STATIC(LogNetworkEventSubsystem, Log, All);

///////////////////////
///// Constructor /////
///////////////////////

UNetworkEventSubsystem::UNetworkEventSubsystem()
	: Super()
{
	bNetworkGameReady = false;
}

//////////////////////////
///// Initialization /////
//////////////////////////

void UNetworkEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConditionalBroadcastNetworkGameReady();
}

void UNetworkEventSubsystem::Deinitialize()
{
	OnNetworkGameReady.Clear();
	OnPlayersUpdated.Clear();
	OnPlayersUpdatedDynamic.Clear();

	Super::Deinitialize();
}

UNetworkEventSubsystem* UNetworkEventSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* lWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (lWorld)
	{
		return lWorld->GetSubsystem<UNetworkEventSubsystem>();
	}

	return nullptr;
}

/////////////////////////
///// Common Actors /////
/////////////////////////

bool IsWorldSafeCommon(const UWorld* InWorld)
{
	return IsValid(InWorld) && !InWorld->IsInSeamlessTravel() && !InWorld->bIsTearingDown;
}

FDelegateHandle UNetworkEventSubsystem::CallAndRegister_OnNetworkGameReady(const UObject* WorldContextObject, FOnNetworkGameReady::FDelegate&& Callback)
{
	if (Callback.IsBound())
	{
		const UWorld* lWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (IsWorldSafeCommon(lWorld))
		{
			UNetworkEventSubsystem* NES = lWorld->GetSubsystem<UNetworkEventSubsystem>();
			if (NES)
			{
				FDelegateHandle ReturnVal = NES->OnNetworkGameReady.Add(Callback);
				if (NES->IsNetworkGameReady())
				{
					Callback.Execute();
				}

				return ReturnVal;
			}
		}
	}

	return FDelegateHandle();
}

FDelegateHandle UNetworkEventSubsystem::CallAndRegister_OnPlayersUpdated(const UObject* WorldContextObject, FOnGameStateEvent::FDelegate&& Callback)
{
	if (Callback.IsBound())
	{
		const UWorld* lWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (IsWorldSafeCommon(lWorld))
		{
			UNetworkEventSubsystem* NES = lWorld->GetSubsystem<UNetworkEventSubsystem>();
			if (NES)
			{
				FDelegateHandle ReturnVal = NES->OnPlayersUpdated.Add(Callback);

				AGameStateBase* WorldGS = lWorld->GetGameState();
				if (IsValid(WorldGS))
				{
					Callback.Execute(WorldGS);
				}

				return ReturnVal;
			}
		}
	}

	return FDelegateHandle();
}

void UNetworkEventSubsystem::ReleaseAll(const void* InObject)
{
	OnNetworkGameReady.RemoveAll(InObject);
	OnPlayersUpdated.RemoveAll(InObject);
}

//////////////////////////////////
///// Common Actors Internal /////
//////////////////////////////////

void UNetworkEventSubsystem::ConditionalBroadcastNetworkGameReady()
{
	if (!bNetworkGameReady)
	{
		bNetworkGameReady = CheckNetworkGameReady();
		if (bNetworkGameReady)
		{
			UE_LOG(LogNetworkEventSubsystem, Log, TEXT("Network Game Ready!"));

			OnNetworkGameReady.Broadcast();
		}
	}
}

bool UNetworkEventSubsystem::CheckNetworkGameReady() const
{
	const UWorld* lWorld = GetWorld();
	if (!IsWorldSafeCommon(lWorld))
	{
		return false;
	}

	// Must have a valid GameState
	const AGameStateBase* WorldGS = lWorld->GetGameState();
	if (!IsValid(WorldGS))
	{
		return false;
	}

	// Every LocalPlayer must have a valid PlayerController and PlayerState
	const TArray<ULocalPlayer*>& AllLocalPlayers = lWorld->GetGameInstance()->GetLocalPlayers();
	for (const ULocalPlayer* LocalPlayer : AllLocalPlayers)
	{
		const APlayerController* LocalPlayer_Controller = LocalPlayer->GetPlayerController(lWorld);
		if (!IsValid(LocalPlayer_Controller))
		{
			return false;
		}

		const APlayerState* LocalPlayer_State = LocalPlayer_Controller->GetPlayerState<APlayerState>();
		if (!IsValid(LocalPlayer_State))
		{
			return false;
		}
	}

	// TODO: Allow GameState to add it's own custom rules?
	return true;
}

//////////////////
///// Arrays /////
//////////////////

void UNetworkEventSubsystem::NotifyPlayerArrayUpdated()
{
	const UWorld* lWorld = GetWorld();
	if (IsWorldSafeCommon(lWorld))
	{
		OnPlayersUpdated.Broadcast(lWorld->GetGameState());
		OnPlayersUpdatedDynamic.Broadcast(lWorld->GetGameState());
	}
}

bool UNetworkEventSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}