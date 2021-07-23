// Copyright Epic Games, Inc. All Rights Reserved.

#include "Levels_v0GameMode.h"
#include "Levels_v0HUD.h"
#include "Levels_v0Character.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"


ALevels_v0GameMode::ALevels_v0GameMode()
	: Super()
{

	PrimaryActorTick.bCanEverTick = true;

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// Sets the healthbar and speed meter as UI
	static ConstructorHelpers::FClassFinder<UUserWidget> HealthBar(TEXT("/Game/FirstPerson/UI/Health_UI"));
	HUDWidgetClass = HealthBar.Class;

	// use our custom HUD class
	HUDClass = ALevels_v0GameMode::StaticClass();

	// add Health Bar UI to viewport
	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void ALevels_v0GameMode::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentState(EGamePlayState::EPlaying);

	MyCharacter = Cast<ALevels_v0Character>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void ALevels_v0GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyCharacter)
	{
		if (FMath::IsNearlyZero(MyCharacter->GetHealth(), 0.001f))
		{
			SetCurrentState(EGamePlayState::EGameOver);
		}
	}
}

EGamePlayState ALevels_v0GameMode::GetCurrentState() const
{
	return CurrentState;
}

void ALevels_v0GameMode::SetCurrentState(EGamePlayState NewState)
{
	CurrentState = NewState;
	HandleNewState(CurrentState);
}

void ALevels_v0GameMode::HandleNewState(EGamePlayState NewState)
{
	switch (NewState)
	{
	case EGamePlayState::EPlaying:
	{
		// do nothing
	}
	break;
	// Unknown/default state
	case EGamePlayState::EGameOver:
	{
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
	}
	break;
	// Unknown/default state
	default:
	case EGamePlayState::EUnknown:
	{
		// do nothing
	}
	break;
	}
}