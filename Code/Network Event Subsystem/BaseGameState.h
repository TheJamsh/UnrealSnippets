#include "GameFramework/GameState.h"
#include "BaseGameState.generated.h"

UCLASS()
class ABaseGameState : public AGameState
{
	GENERATED_BODY()
public:
	// Constructor
	ABaseGameState(const FObjectInitializer& OI);

	// Overrides
	virtual void PostNetInit() override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
};
