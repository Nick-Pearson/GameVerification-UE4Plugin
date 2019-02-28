#include "VerificationEntityComponent.h"
#include "IGameVerification.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"

UVerificationEntityComponent::UVerificationEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UVerificationEntityComponent::InitializeComponent()
{
	Super::InitializeComponent();

	m_PluginInterface = &IGameVerification::Get();

	UGameInstance* GI = GetWorld()->GetGameInstance<UGameInstance>();
	m_SessionID = m_PluginInterface->GetSessionID(GI);
}

void UVerificationEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bReplicates || GetOwner()->HasAuthority())
	{
		m_EntityID = m_PluginInterface->EntitySpawned(m_SessionID, EntityType);
	}
}


void UVerificationEntityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	m_PluginInterface->EntityDestroyed(m_SessionID, m_EntityID);
}

void UVerificationEntityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVerificationEntityComponent, m_EntityID);
}

void UVerificationEntityComponent::OnRep_EntityID()
{
	if (bReplicates || !GetOwner()->HasAuthority())
	{
		 m_PluginInterface->EntitySpawned(m_SessionID, m_EntityID);
	}
}
