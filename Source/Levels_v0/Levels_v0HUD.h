// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Levels_v0HUD.generated.h"

UCLASS()
class ALevels_v0HUD : public AHUD
{
	GENERATED_BODY()

public:
	ALevels_v0HUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	UPROPERTY(EditAnywhere, Category = "Health")
		TSubclassOf<class UUserWidget> HUDWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "Health")
		class UUserWidget* CurrentWidget;


};

