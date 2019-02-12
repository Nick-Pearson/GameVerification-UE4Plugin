#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

// Runtime module for performing game verification
class IGameVerification : public IModuleInterface
{

public:

	virtual void StartVerificationSession(UGameInstance* GameInstance) = 0;

	virtual void EndVerificationSession(UGameInstance* GameInstance) = 0;

	static inline IGameVerification& Get()
	{
		return FModuleManager::LoadModuleChecked< IGameVerification >( "GameVerification" );
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "GameVerification" );
	}
};

