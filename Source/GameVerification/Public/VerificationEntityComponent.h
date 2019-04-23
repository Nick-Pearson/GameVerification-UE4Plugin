#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VerificationEntity.h"

#include "VerificationEntityComponent.generated.h"

class UVerificationEntity;

UCLASS(BlueprintType, ClassGroup=(Verification))
class GAMEVERIFICATION_API UVerificationEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity")
	FString EntityType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity", ReplicatedUsing=OnRep_EntityData, meta = (ShowOnlyInnerProperties))
	UVerificationEntity* EntityData;

public:
	// Sets default values for this component's properties
	UVerificationEntityComponent();

	FORCEINLINE void UpdateProperty(const FString& Name, bool Value) { if (EntityData) EntityData->UpdateProperty(Name, Value); }
	FORCEINLINE void UpdateProperty(const FString& Name, int Value) { if (EntityData) EntityData->UpdateProperty(Name, Value); }

protected:

	UFUNCTION()
	void OnRep_EntityData();

	void InitializeComponent() override;

	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;


};
