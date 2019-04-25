#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameVerificationSettings.generated.h"

UCLASS(config = Game)
class UGameVerificationSettings : public UObject
{
	GENERATED_BODY()

public:

	// hostname of the monitor
	UPROPERTY(config, EditAnywhere, Category = "Checking")
	FString MontiorHost = "localhost";

	// location of the VEN file for the project
	UPROPERTY(config, EditAnywhere, Category = "Checking")
	int MonitorPort = 8080;

	// location of the VEN file for the project
	UPROPERTY(config, EditAnywhere, Category = "Checking")
	FString VENFilePath = "Verification/model.ven";


	UPROPERTY(config, EditAnywhere, Category = "Driving")
	bool EnableBDIDuringPlay;

	// triggering event for generated BDI plans
	UPROPERTY(config, EditAnywhere, Category = "Driving", meta = (EditCondition = "EnableBDIDuringPlay"))
	FString DefaultPlanTrigger = "+!start";

	// location of the PLN file for the project
	UPROPERTY(config, EditAnywhere, Category = "Driving", meta = (EditCondition = "EnableBDIDuringPlay"))
	FString PLNFilePath = "Verification/model.pln";

	UPROPERTY(config, EditAnywhere, Category = "Driving", meta = (EditCondition = "EnableBDIDuringPlay"))
	FString ASFilePath = "Verification/model.mas2j";




};
