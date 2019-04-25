#include "GameVerification.h"

#include "api.h"
#include "verificationclient.h"
#include "model/verificationmodel.h"
#include "verificationtypes.h"
#include "driving/ibdiinstance.h"
#include "GameVerificationSettings.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"

#define LOCTEXT_NAMESPACE "GameVerification"

using namespace GameVerification;

void FVerificationTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	API::Event e{ EventType::FRAME_BOUNDRY };
	PluginPtr->SendEvent(sessionID, &e, sizeof(e));

	PluginPtr->ExecuteBDIActions();
}

FString FVerificationTickFunction::DiagnosticMessage()
{
	return "Verification TickFunc";
}

void FSessionData::Initialise(GameVerification::SessionID sessionID, UGameInstance* GameInstance, FGameVerification* PluginHandle)
{
	if (!ensureMsgf(GameInstance, TEXT("Game instance was nullptr when initialising verification session"))) return;

	InstancePtr = GameInstance;

	if (TickFunc.IsTickFunctionRegistered())
	{
		TickFunc.UnRegisterTickFunction();
	}

	TickFunc.PluginPtr = PluginHandle;
	TickFunc.sessionID = sessionID;

	TickFunc.bCanEverTick = true;
	TickFunc.bStartWithTickEnabled = true;
	TickFunc.bAllowTickOnDedicatedServer = true;
	TickFunc.TickGroup = TG_PostUpdateWork;
	
	UWorld* World = GameInstance->GetWorld();
	ULevel* Level = World ? World->PersistentLevel : nullptr;

	if (!Level)
	{
		return;
	}

	TickFunc.RegisterTickFunction(Level);
}

IMPLEMENT_MODULE(FGameVerification, GameVerification)

void FGameVerification::StartupModule()
{
	LoadDLL();
	
#if WITH_EDITOR
	// register settings
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");

	ISettingsSectionPtr SettingsSection = SettingsModule.RegisterSettings("Project", "Plugins", "GameVerification",
		LOCTEXT("UMSettingsName", "Game Verification"),
		LOCTEXT("UMSettingsDescription", "Configure the Game Verification plugin"),
		GetMutableDefault<UGameVerificationSettings>()
	);

	BeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FGameVerification::OnBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddRaw(this, &FGameVerification::OnEndPIE);
#endif // WITH_EDITOR

	config = new Config();

	SetupConfigFromSettings();
}

void FGameVerification::ShutdownModule()
{
	delete config;
	delete client;

	config = nullptr;
	client = nullptr;

	if (DLLHandle)
	{
		FPlatformProcess::FreeDllHandle(DLLHandle);
	}
	DLLHandle = nullptr;


#if WITH_EDITOR
	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "GameVerification");
	}

	FEditorDelegates::PostPIEStarted.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
#endif
}

SessionID FGameVerification::StartVerificationSession(UGameInstance* GameInstance)
{
	if (!client) client = CreateClient();
	
	SessionType type = GameInstance->IsDedicatedServerInstance() ? SessionType::Server : SessionType::Client;
	SessionID id = client->sessionStart(type);

	if (id == INVALID_SESSION) return INVALID_SESSION;

	CurrentSession = id;

	FSessionData& data = SessionData.Add(id);
	data.Initialise(id, GameInstance, this);

	SessionIDMap.Add(GameInstance, id);

	return id;
}

void FGameVerification::EndVerificationSession(SessionID id)
{
	if (!client) return;

	API::SessionEvent e{ id, EventType::SESSION_END };
	client->sendEvent(&e, sizeof(e));

	FSessionData* data = SessionData.Find(id);

	if (!data) return;

	SessionIDMap.Remove(data->InstancePtr);
	SessionData.Remove(id);
	
}

void FGameVerification::UpdateVerificationSession(GameVerification::SessionID id, UGameInstance* GameInstance)
{
	// TODO: Broadcast a connect/disconnect event
	FSessionData* data = SessionData.Find(id);

	if (!data) return;

	data->Initialise(id, GameInstance, this);
}

FVerificationEntityID FGameVerification::EntitySpawned(GameVerification::SessionID session, FString TypeName)
{
	FVerificationEntityID id;

	SwitchToSession(session);

	if (client)
	{
		id = client->entitySpawned(TCHAR_TO_ANSI(*TypeName));
	}

	return id;
}

void FGameVerification::EntitySpawned(GameVerification::SessionID session, const FVerificationEntityID& id)
{
	if (!client) return;

	SwitchToSession(session);

	API::EntityEvent e{ id.GetValue(), EventType::ENTITY_SPAWNED };
	client->sendEvent(&e, sizeof(e));
}

void FGameVerification::EntityDestroyed(GameVerification::SessionID session, const FVerificationEntityID& id)
{
	if (!client) return;

	SwitchToSession(session);

	API::EntityEvent e{ id.GetValue(), EventType::ENTITY_DESTROYED };
	client->sendEvent(&e, sizeof(e));
}

void FGameVerification::PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, bool value)
{
	PropertyChanged(session, id, prop, PropertyValue(value));
}

void FGameVerification::PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, int value)
{
	PropertyChanged(session, id, prop, PropertyValue(value));
}

void FGameVerification::PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, const PropertyValue& value)
{
	if (!client) return;

	SwitchToSession(session);

	API::PropertyChangedEvent e{ id.GetValue(), GetPropertyIDFromString(id.EntityType, prop), value };

	if (e.propertyID == INVALID_PROPERTY)
	{
		return;
	}

	client->sendEvent(&e, sizeof(e));

	if (!bdiInstance) return;

	List<CustomString> beliefs = client->GetModel()->getRelevantBeliefs(e.entityID, e.propertyID, e.value);

	for (const CustomString& bel : beliefs)
	{
		bdiInstance->updateBelief(bel.raw_data());
	}
}

void FGameVerification::SubentityChanged(SessionID session, const FVerificationEntityID& thisEntity, const FString& prop, const FVerificationEntityID& otherEntity)
{
	if (!client) return;

	SwitchToSession(session);

	API::SubentityEvent e{ thisEntity.GetValue(), GetPropertyIDFromString(thisEntity.EntityType, prop), otherEntity.GetValue() };

	if (e.propertyID == INVALID_PROPERTY)
	{
		return;
	}

	client->sendEvent(&e, sizeof(e));
}

void FGameVerification::CreateAgent(const FString& AgentName, const FString& ASLFilepath, const FVerificationEntityID& linkedEntity)
{
	if (!bdiInstance) return;

	bdiInstance->createAgent(TCHAR_TO_ANSI(*AgentName), TCHAR_TO_ANSI(*ASLFilepath));

	if(linkedEntity.IsValid())
		bdiInstance->linkEntity(linkedEntity.GetValue(), TCHAR_TO_ANSI(*AgentName));
}

SessionID FGameVerification::GetSessionID(const UGameInstance* GameInstance)
{
	SessionID id = INVALID_SESSION;

	SessionID* idPtr = SessionIDMap.Find(GameInstance);
	if (idPtr)
	{
		id = *idPtr;
	}

	return id;
}

void FGameVerification::SendEvent(SessionID session, API::Event* eventPtr, size_t eventSize)
{
	if (!client) return;

	SwitchToSession(session);
	client->sendEvent(eventPtr, eventSize);
}

void FGameVerification::ExecuteBDIActions()
{
	if (!bdiInstance) return;

	bdiInstance->executeActions();
}

void FGameVerification::SwitchToSession(GameVerification::SessionID NewSession)
{
	if (CurrentSession == NewSession || !client) return;

	API::SessionEvent e{ NewSession, EventType::SET_ACTIVE_SESSION };
	client->sendEvent(&e, sizeof(e));
	CurrentSession = NewSession;
}

PropertyID FGameVerification::GetPropertyIDFromString(EntityType entity, const FString& propertyName)
{
	FPropertyCacheEntry* entry = PropertyNameCache.Find(entity);

	if (!entry)
	{
		entry = &PropertyNameCache.Add(entity);
	}

	PropertyID* id = entry->Entries.Find(propertyName);

	if(!id)
	{
		if (!client) return INVALID_PROPERTY;

		id = &entry->Entries.Add(propertyName, client->GetModel()->getPropertyID(entity, TCHAR_TO_ANSI(*propertyName)));
	}

	return *id;
}

VerificationClient* FGameVerification::CreateClient()
{
	SetupConfigFromSettings();
	VerificationClient* c = new VerificationClient(config);
	c->connect();
	return c;
}

void FGameVerification::OnBeginPIE(const bool isSimulating)
{
	const UGameVerificationSettings* settings = GetDefault<UGameVerificationSettings>();
	if (!settings || !settings->EnableBDIDuringPlay) return;

	bdiInstance = GameVerification::CreateBDIInstance(TCHAR_TO_ANSI(*(FPaths::ProjectDir() / settings->ASFilePath)));
}

void FGameVerification::OnEndPIE(const bool isSimulating)
{
	if (!bdiInstance) return;

	bdiInstance.reset();
}

void FGameVerification::LoadDLL()
{
#if PLATFORM_LINUX
	return;
#elif PLATFORM_WINDOWS
	FString PlatformDir = "Win64";
#elif PLATFORM_MAC
	FString PlatformDir = "Mac";
#else
#error "GameVerification:: Platform not supported"
#endif
	FString BaseDir = IPluginManager::Get().FindPlugin("GameVerification")->GetBaseDir();
	FString DLLName = FString("VerificationRuntime.") + FPlatformProcess::GetModuleExtension();
	FString DLLPath = FPaths::Combine(BaseDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("GameVerificationLibrary"), *PlatformDir, *DLLName);

	DLLHandle = FPlatformProcess::GetDllHandle(*DLLPath);

	check(DLLHandle);
}

void FGameVerification::SetupConfigFromSettings()
{
	const UGameVerificationSettings* settings = GetDefault<UGameVerificationSettings>();

	if (!config || !settings) return;

	config->serverHost = TCHAR_TO_ANSI(*settings->MontiorHost);
	config->serverPort = settings->MonitorPort;
	
	config->modelFile = TCHAR_TO_ANSI(*(FPaths::ProjectDir() / settings->VENFilePath));
	config->plnFile = TCHAR_TO_ANSI(*(FPaths::ProjectDir() / settings->PLNFilePath));
}

#undef LLOCTEXT_NAMESPACE