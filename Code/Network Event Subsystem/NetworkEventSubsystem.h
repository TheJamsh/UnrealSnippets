#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "NetworkEventSubsystem.generated.h"

/*
* Network Event Subsystem
* Contains a series of useful callbacks for network games.
*/
UCLASS(NotBlueprintable, BlueprintType)
class UNetworkEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// Constructor
	UNetworkEventSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Accessor
	UFUNCTION(BlueprintCallable, Category = "Network Events", meta = (CompactNodeTitle = "Network Event System", WorldContext = "WorldContextObject"))
	static UNetworkEventSubsystem* Get(const UObject* WorldContextObject);

	// Event Types
	DECLARE_MULTICAST_DELEGATE(FOnNetworkGameReady);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateEvent, AGameStateBase*);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateEventDynamic, AGameStateBase*, GameState);

	/*
	* Binds (or executes) a callback when common networked actors have been received and their states are updated.
	* 
	* - The GameState
	* - All Local Player Controllers
	* - All Local Player States
	*/
	static FDelegateHandle CallAndRegister_OnNetworkGameReady(const UObject* WorldContextObject, FOnNetworkGameReady::FDelegate&& Callback);
	static FDelegateHandle CallAndRegister_OnPlayersUpdated(const UObject* WorldContextObject, FOnGameStateEvent::FDelegate&& Callback);

	void ReleaseAll(const void* InObject);

	// Accessors
	void NotifyGameNetworkActorUpdate() { ConditionalBroadcastNetworkGameReady(); }
	void NotifyPlayerArrayUpdated();

	bool IsNetworkGameReady() const { return bNetworkGameReady; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/*
	* NB: Blueprint-Use Only!
	* Broadcast when the Player Array is updated.
	*/
	UPROPERTY(Transient, BlueprintAssignable, Category = "Network Events", meta = (DisplayName = "On Players Updated"))
	FOnGameStateEventDynamic OnPlayersUpdatedDynamic;

private:
	void ConditionalBroadcastNetworkGameReady();
	bool CheckNetworkGameReady() const;

	FOnNetworkGameReady OnNetworkGameReady;
	FOnGameStateEvent OnPlayersUpdated;

	bool bNetworkGameReady;
};
