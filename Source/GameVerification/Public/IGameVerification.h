#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "verificationtypes.h"

#include "IGameVerification.generated.h"

class UGameInstance;

// USTRUCT version of entity ID for replicating
USTRUCT()
struct FVerificationEntityID
{
	GENERATED_BODY()

public:
	UPROPERTY()
	uint32 EntityType = INVALID_ENTITY;

	UPROPERTY()
	uint32 Idx = 0;

	FVerificationEntityID& operator=(const GameVerification::EntityID& id)
	{
		EntityType = id.Type;
		Idx = id.Idx;
		return *this;
	}

	bool operator==(const FVerificationEntityID& other) const
	{
		return EntityType == other.EntityType && Idx == other.Idx;
	}

	GameVerification::EntityID GetValue() const
	{
		GameVerification::EntityID id;
		id.Type = (GameVerification::EntityType)EntityType;
		id.Idx = Idx;

		return id;
	}

	inline bool IsValid() const { return EntityType != INVALID_ENTITY; }
};

// Runtime module for performing game verification
class IGameVerification : public IModuleInterface
{

public:

	virtual GameVerification::SessionID StartVerificationSession(UGameInstance* GameInstance) = 0;
	virtual void EndVerificationSession(GameVerification::SessionID id) = 0;

	// Called when the active level is changed, need to update the session state
	virtual void UpdateVerificationSession(GameVerification::SessionID id, UGameInstance* GameInstance) = 0;

	virtual FVerificationEntityID EntitySpawned(GameVerification::SessionID session, FString TypeName) = 0;
	virtual void EntitySpawned(GameVerification::SessionID session, const FVerificationEntityID& id) = 0;

	virtual void EntityDestroyed(GameVerification::SessionID session, const FVerificationEntityID& id) = 0;

	virtual void PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, bool value) = 0;
	virtual void PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, int value) = 0;

	virtual void SubentityChanged(GameVerification::SessionID session, const FVerificationEntityID& thisEntity, const FString& prop, const FVerificationEntityID& otherEntity) = 0;

	virtual void GlobalPropertyChanged(GameVerification::SessionID session, const FString& prop, bool value) = 0;
	virtual void GlobalPropertyChanged(GameVerification::SessionID session, const FString& prop, int value) = 0;

	virtual void GlobalSubentityChanged(GameVerification::SessionID session, const FString& prop, const FVerificationEntityID& subentity) = 0;

	virtual void CreateAgent(const FString& AgentName, const FString& ASLFilepath, const FVerificationEntityID& linkedEntity) = 0;

	virtual GameVerification::SessionID GetSessionID(const UGameInstance* GameInstance) = 0;

	static inline IGameVerification& Get()
	{
		return FModuleManager::LoadModuleChecked< IGameVerification >( "GameVerification" );
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "GameVerification" );
	}
};

