#include "CoreMinimal.h"
#include "IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "GameVerification.h"

#include "verificationtypes.h"
#include "config.h"

using namespace GameVerification;

class FGameVerification : public IGameVerification
{
	Config* config = nullptr;
	void* DLLHandle = nullptr;

public:
	virtual void StartupModule() override
	{
		LoadDLL();

		config = new Config("");
	}


	virtual void ShutdownModule() override
	{
		delete config;

		if (DLLHandle)
		{
			FPlatformProcess::FreeDllHandle(DLLHandle);
		}
		DLLHandle = nullptr;
	}

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

IMPLEMENT_MODULE( FGameVerification, GameVerification )