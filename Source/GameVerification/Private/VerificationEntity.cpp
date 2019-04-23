#include "VerificationEntity.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"


void UVerificationEntity::Initialise(bool doesReplicate, UWorld* WorldPtr)
{
	m_Replicates = doesReplicate;
	m_PluginInterface = &IGameVerification::Get();

	UGameInstance* GI = WorldPtr->GetGameInstance<UGameInstance>();
	m_SessionID = m_PluginInterface->GetSessionID(GI);

	if (m_EntityID.IsValid())
	{
		m_PluginInterface->EntitySpawned(m_SessionID, m_EntityID);
	}
}

void UVerificationEntity::OnBeginPlay()
{
	m_EntityID = m_PluginInterface->EntitySpawned(m_SessionID, EntityType);
}

void UVerificationEntity::OnEndPlay()
{
	m_PluginInterface->EntityDestroyed(m_SessionID, m_EntityID);
}

void UVerificationEntity::UpdateProperty(const FString& Name, bool Value)
{
	bool* Cached = CachedBoolValues.Find(Name);
	if (Cached)
	{
		if (*Cached == Value) return;
		*Cached = Value;
	}
	else
	{
		CachedBoolValues.Add(Name, Value);
	}

	m_PluginInterface->PropertyChanged(m_SessionID, m_EntityID, Name, Value);
}

void UVerificationEntity::UpdateProperty(const FString& Name, int Value)
{
	int* Cached = CachedIntValues.Find(Name);
	if (Cached)
	{
		if (*Cached == Value) return;
		*Cached = Value;
	}
	else
	{
		CachedIntValues.Add(Name, Value);
	}

	m_PluginInterface->PropertyChanged(m_SessionID, m_EntityID, Name, Value);
}

void UVerificationEntity::OnRep_EntityID()
{
	if(m_PluginInterface)
		m_PluginInterface->EntitySpawned(m_SessionID, m_EntityID);
}

void UVerificationEntity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UVerificationEntity, m_EntityID);
}