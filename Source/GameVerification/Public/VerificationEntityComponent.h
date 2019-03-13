#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "verificationtypes.h"
#include "IGameVerification.h"

#include "VerificationEntityComponent.generated.h"

UCLASS(BlueprintType, ClassGroup=(Verification))
class GAMEVERIFICATION_API UVerificationEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity")
	FString EntityType;

public:
	// Sets default values for this component's properties
	UVerificationEntityComponent();

	void UpdateProperty(const FString& Name, bool Value);

protected:

	void InitializeComponent() override;

	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	IGameVerification* m_PluginInterface;

	UFUNCTION()
	void OnRep_EntityID();

	UPROPERTY(Transient, ReplicatedUsing=OnRep_EntityID)
	FVerificationEntityID m_EntityID;

	GameVerification::SessionID m_SessionID;

	TMap<FString, bool> CachedValues;

};
