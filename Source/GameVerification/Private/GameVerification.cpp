#include "CoreMinimal.h"
#include "IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "GameVerification.h"

#include "verificationtypes.h"
#include "verificationclient.h"
#include "config.h"
#include "api.h"

using namespace GameVerification;

class FGameVerification : public IGameVerification
{
	Config* config = nullptr;
	VerificationClient* client = nullptr;

	void* DLLHandle = nullptr;

	TMap <UGameInstance*, SessionID> SessionMap;
	SessionID CurrentSession = INVALID_SESSION;


public:
	virtual void StartupModule() override
	{
		LoadDLL();

		config = new Config();
	}

	virtual void ShutdownModule() override
	{
		delete config;
		if (client)
		{
			client->disconnect(true);
			delete client;
		}

		SessionMap.Empty();

		if (DLLHandle)
		{
			FPlatformProcess::FreeDllHandle(DLLHandle);
		}
		DLLHandle = nullptr;
	}

	void StartVerificationSession(UGameInstance* GameInstance) override;

	void EndVerificationSession(UGameInstance* GameInstance) override;

private:

	VerificationClient* CreateClient();

	void LoadDLL()
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

};

void FGameVerification::StartVerificationSession(UGameInstance* GameInstance)
{
	if (!client) client = CreateClient();
	
	SessionType type = GameInstance->IsDedicatedServerInstance() ? SessionType::Server : SessionType::Client;
	API::SessionStartEvent e{ type };
	client->sendEvent(&e, sizeof(e));
}

void FGameVerification::EndVerificationSession(UGameInstance* GameInstance)
{
	SessionID* id = SessionMap.Find(GameInstance);

	if (!id) return;

	if (client)
	{
		API::SessionEndEvent e{ *id };
		client->sendEvent(&e, sizeof(e));
	}

	SessionMap.Remove(GameInstance);
}

IMPLEMENT_MODULE(FGameVerification, GameVerification)

VerificationClient* FGameVerification::CreateClient()
{
	VerificationClient* c = new VerificationClient();
	c->connect(config->serverHost, config->serverPort);
	return c;
}
