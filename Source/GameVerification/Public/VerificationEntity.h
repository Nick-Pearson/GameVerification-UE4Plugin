#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "verificationtypes.h"
#include "IGameVerification.h"

#include "VerificationEntity.generated.h"

UCLASS(BlueprintType)
class GAMEVERIFICATION_API UVerificationEntity : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "VerificationEntity")
	FString EntityType;

#if VERIFICATION_ENABLED
	void Initialise(bool doesReplicate, UWorld* WorldPtr);

	void OnBeginPlay();
	void OnEndPlay();

	void UpdateProperty(const FString& Name, bool Value);
	void UpdateProperty(const FString& Name, int Value);

#else
	FORCE_INLINE void UpdateProperty(const FString& Name, bool Value) {}
	FORCE_INLINE void UpdateProperty(const FString& Name, int Value) {}
#endif

	bool IsSupportedForNetworking() const override
	{
		return true;
	}

private:

	bool m_Replicates = false;

	IGameVerification* m_PluginInterface;

	UFUNCTION()
	void OnRep_EntityID();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EntityID)
	FVerificationEntityID m_EntityID;

	GameVerification::SessionID m_SessionID;

	TMap<FString, bool> CachedBoolValues;
	TMap<FString, int> CachedIntValues;
};
