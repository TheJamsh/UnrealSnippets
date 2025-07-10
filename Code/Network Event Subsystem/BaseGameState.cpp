#include "BaseGameState.h"
#include "Core/Subsystems/NetworkEventSubsystem.h"

///////////////////////
///// Constructor /////
///////////////////////

ABaseGameState::ABaseGameState(const FObjectInitializer& OI)
	: Super(OI)
{}

/////////////////////
///// Overrides /////
/////////////////////

void ABaseGameState::PostNetInit()
{
	Super::PostNetInit();

	if (UNetworkEventSubsystem* NetSubsystem = UNetworkEventSubsystem::Get(this))
	{
		NetSubsystem->NotifyGameNetworkActorUpdate();
	}
}

void ABaseGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (UNetworkEventSubsystem* NetSubsystem = UNetworkEventSubsystem::Get(this))
	{
		NetSubsystem->NotifyPlayerArrayUpdated();
	}
}

void ABaseGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (UNetworkEventSubsystem* NetSubsystem = UNetworkEventSubsystem::Get(this))
	{
		NetSubsystem->NotifyPlayerArrayUpdated();
	}
}