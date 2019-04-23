#include "IGameVerification.h"
#include "IPluginManager.h"
#include "Editor.h"
#include "Paths.h"

#include "verificationtypes.h"
#include "verificationclient.h"
#include "config.h"

class FGameVerification;

struct FVerificationTickFunction : public FTickFunction
{
	FGameVerification* PluginPtr = nullptr;
	GameVerification::SessionID sessionID;

private:
	void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	FString DiagnosticMessage() override;
};


struct FSessionData
{
public:

	// setup struct
	void Initialise(GameVerification::SessionID sessionID, UGameInstance* GameInstance, class FGameVerification* PluginHandle);

	UGameInstance* InstancePtr;

	FVerificationTickFunction TickFunc;
};


class FGameVerification : public IGameVerification
{
	GameVerification::Config* config = nullptr;
	GameVerification::VerificationClient* client = nullptr;

	void* DLLHandle = nullptr;

	GameVerification::SessionID CurrentSession = INVALID_SESSION;
	
public:
	void StartupModule() override;
	void ShutdownModule() override;

	GameVerification::SessionID StartVerificationSession(UGameInstance* GameInstance) override;
	void EndVerificationSession(GameVerification::SessionID id) override;
	void UpdateVerificationSession(GameVerification::SessionID id, UGameInstance* GameInstance) override;

	FVerificationEntityID EntitySpawned(GameVerification::SessionID session, FString TypeName) override;
	void EntitySpawned(GameVerification::SessionID session, const FVerificationEntityID& id) override;

	void EntityDestroyed(GameVerification::SessionID session, const FVerificationEntityID& id) override;

	void PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, bool value) override;
	void PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, int value) override;
	void PropertyChanged(GameVerification::SessionID session, const FVerificationEntityID& id, const FString& prop, const struct GameVerification::PropertyValue& value);

	GameVerification::SessionID GetSessionID(const UGameInstance* GameInstance) override;

	void SendEvent(GameVerification::SessionID session, GameVerification::API::Event* eventPtr, size_t eventSize);

	inline GameVerification::VerificationClient* GetVerificationClient() const { return client; }

private:

	void SwitchToSession(GameVerification::SessionID NewSession);

	GameVerification::PropertyID GetPropertyIDFromString(GameVerification::EntityType entity, const FString& propertyName);

	TMap<GameVerification::SessionID, FSessionData> SessionData;
	TMap<UGameInstance*, GameVerification::SessionID> SessionIDMap;

	struct FPropertyCacheEntry 
	{
		TMap<FString, GameVerification::PropertyID> Entries;
	};
	TMap<GameVerification::EntityType, FPropertyCacheEntry> PropertyNameCache;

	GameVerification::VerificationClient* CreateClient();

	void LoadDLL();

};