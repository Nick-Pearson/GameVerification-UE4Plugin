#include "VerificationEntityComponent.h"
#include "IGameVerification.h"
#include "UnrealNetwork.h"

#include "Engine/ActorChannel.h"

UVerificationEntityComponent::UVerificationEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UVerificationEntityComponent::UpdatePropertyBool_BP(const FString& Name, bool Value)
{
	UpdateProperty(Name, Value);
}

void UVerificationEntityComponent::UpdatePropertyInt_BP(const FString& Name, int Value)
{
	UpdateProperty(Name, Value);
}

void UVerificationEntityComponent::OnRep_EntityData()
{
	if (EntityData)
	{
		EntityData->Params = Params;
		EntityData->Initialise(bReplicates, GetWorld());

		if (HasBegunPlay())
			EntityData->OnBeginPlay();
	}
}

void UVerificationEntityComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!bReplicates || GetOwner()->HasAuthority())
	{
		EntityData = NewObject<UVerificationEntity>(GetOwner());
		EntityData->Params = Params;
		EntityData->Initialise(bReplicates, GetWorld());
	}
}

void UVerificationEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (EntityData && (!bReplicates || GetOwner()->HasAuthority()))
	{
		EntityData->OnBeginPlay();
	}
}


void UVerificationEntityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EntityData) EntityData->OnEndPlay();
}

void UVerificationEntityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVerificationEntityComponent, EntityData);
}

bool UVerificationEntityComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (EntityData)
		WroteSomething |= Channel->ReplicateSubobject(EntityData, *Bunch, *RepFlags);

	return WroteSomething;
}