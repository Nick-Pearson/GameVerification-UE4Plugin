#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "verificationtypes.h"
#include "IGameVerification.h"

#include "VerificationEntity.generated.h"

USTRUCT(BlueprintType)
struct FVerificationEntityParams
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity")
	FString EntityType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity")
	bool SpawnBDIAgent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VerificationEntity", meta = (EditCondition = "SpawnBDIAgent"))
	FString BDIAgentName;
};


UCLASS(BlueprintType)
class GAMEVERIFICATION_API UVerificationEntity : public UObject
{
	GENERATED_BODY()
	
public:

	FVerificationEntityParams Params;

public:

	UFUNCTION(BlueprintCallable, Category = "VerificationEntity", meta = (DisplayName = "Update Property (bool)"))
	void UpdatePropertyBool_BP(const FString& Name, bool Value);

	UFUNCTION(BlueprintCallable, Category = "VerificationEntity", meta = (DisplayName = "Update Property (int)"))
	void UpdatePropertyInt_BP(const FString& Name, int Value);

	inline FVerificationEntityID GetEntityID() const { return m_EntityID; }

#if VERIFICATION_ENABLED

	void Initialise(bool doesReplicate, UWorld* WorldPtr);

	void OnBeginPlay();
	void OnEndPlay();

	void UpdateProperty(const FString& Name, bool Value);
	void UpdateProperty(const FString& Name, int Value);
	void UpdateSubentity(const FString& Name, const FVerificationEntityID& Subentity);

	FORCEINLINE void UpdateGlobalProperty(const FString& Name, bool Value) { m_PluginInterface->GlobalPropertyChanged(m_SessionID, Name, Value); }
	FORCEINLINE void UpdateGlobalProperty(const FString& Name, int Value) { m_PluginInterface->GlobalPropertyChanged(m_SessionID, Name, Value); }
	FORCEINLINE void UpdateGlobalSubentity(const FString& Name, const FVerificationEntityID& Subentity) { m_PluginInterface->GlobalSubentityChanged(m_SessionID, Name, Subentity); }

#else

	FORCE_INLINE void Initialise(bool doesReplicate, UWorld* WorldPtr) {}

	FORCE_INLINE void OnBeginPlay() {}
	FORCE_INLINE void OnEndPlay() {}

	FORCE_INLINE void UpdateProperty(const FString& Name, bool Value) {}
	FORCE_INLINE void UpdateProperty(const FString& Name, int Value) {}
	FORCE_INLINE void UpdateSubentity(const FString& Name, const FVerificationEntityID& subentity) {}

	FORCE_INLINE void UpdateGlobalProperty(const FString& Name, bool Value) {  }
	FORCE_INLINE void UpdateGlobalProperty(const FString& Name, int Value) { }
	FORCE_INLINE void UpdateGlobalSubentity(const FString& Name, const FVerificationEntityID& Subentity) {  }

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
	TMap<FString, FVerificationEntityID> CachedSubentityValues;
};
