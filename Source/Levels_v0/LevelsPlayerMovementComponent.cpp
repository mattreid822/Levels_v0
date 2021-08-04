// Fill out your copyright notice in the Description page of Project Settings.

#include "Levels_v0Character.h"
#include "LevelsPlayerMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Classes/Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Classes/GameFramework/Controller.h"
#include "Engine/Classes/Components/CapsuleComponent.h"

ULevelsPlayerMovementComponent::ULevelsPlayerMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULevelsPlayerMovementComponent::WallMovementCheck()
{
	if (bWallRunEnabled)
	{
		WallRunUpdate();
	}
	if (bWallClimbEnabled)
	{
		WallClimbUpdate();
	}
	if (bMantleCheckEnabled)
	{
		if (MantleCheck())
		{
			MantleStart();
		}
	}
	if (bMantleEnabled) {
		MantleMovement();
	}
	if (bSprintEnabled) {
		SprintUpdate();
	}
	if (bSlidingEnabled)
	{
		SlideUpdate();
	}
}

void ULevelsPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(WallRunTimerHandle, this, &ULevelsPlayerMovementComponent::WallMovementCheck, 0.0167f, true);

	//sets a timer to check if the camera rotation should be changed based on the custom movement mode. Could just be called in wall movement check but its here for now
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("MovementCamera"), MovementCameraRoll);
	GetWorld()->GetTimerManager().SetTimer(CameraTimerHandle, TimerDel, 0.0167f, true);

	//save default values so we can change them back
	DefaultGravity = GravityScale;
	DefaultGroundFriction = GroundFriction;
	DefaultBrakingDeceleration = BrakingDecelerationWalking;
	DefaultWalkSpeed = MaxWalkSpeed;
	DefaultCrouchSpeed = MaxWalkSpeedCrouched;



}

void ULevelsPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UE_LOG(LogTemp, Warning, TEXT("Test: %d"), CustomMovementMode);
	UE_LOG(LogTemp, Warning, TEXT("Bool value is: %s"), bSlidingEnabled ? "e" : "f a l s e");
}

void ULevelsPlayerMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//PreviousCustomMode = CustomMovementMode;
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Walking && IsFalling())
	{
		//EnableWallRun();
		EnableWallClimb();
		EnableSlide();
		EnableSprint();
		SprintJump();
		if (PreviousCustomMode == MOVE_Sprint)
			bWantsToSprint = true;
		WallRunEnd(0.35f);
		WallClimbEnd(0.0f);
		SlideEnd(false);
	}
	else if (PreviousMovementMode == MOVE_Falling && IsWalking())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Queued Movement!!"));
		//CheckQueuedMovement();
	}
}

void ULevelsPlayerMovementComponent::ProcessLanded(const FHitResult & Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);
	DisableWallRun();
	DisableWallClimb();
	DisableSlide();
	DisableSprint();
	WallRunEnd(0.35f);
	WallClimbEnd(0.0f);
	SprintEnd();
	SlideEnd(false);
	CheckQueuedMovement();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Camera Shake!!"));
	//GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake, 1, ECameraAnimPlaySpace::CameraLocal, FRotator(0, 0, 0));
	GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake);
}

void ULevelsPlayerMovementComponent::OnJump()
{
;
	if (CustomMovementMode == MOVE_CustomNone)
	{
		if (!IsFalling())
		{
			EnableWallRun();
			EnableWallClimb();
			EnableSprint();
			EnableSlide();
			GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake);
		}
	}
	else
	{
		WallRunJump();
		LedgeGrabJump();
		SlideJump();
		CrouchJump();
		SprintJump();
	}
	//GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake);
}

//bool ULevelsPlayerMovementComponent::DoJump(bool bReplayingMoves)
//{
	//if (CustomMovementMode == MOVE_CustomNone && IsFalling())
	//{
	//	if (IsFalling())
	//	{
	//		EnableWallRun();
	//		EnableWallClimb();
	//		EnableSprint();
	//		EnableSlide();
	//		SlideJump();
	//		CrouchJump();
	//		SprintJump();
	//	}
	//}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Camera Shake!!"));
	//GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake);
	//GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(JumpLandShake);
	//return Super::DoJump(bReplayingMoves);
//}

bool ULevelsPlayerMovementComponent::SetCustomMovementMode(uint8 NewCustomMovementMode)
{
	if (CustomMovementMode == NewCustomMovementMode)
	{
		return false;
	}
	else
	{
		//if (NewCustomMovementMode == MOVE_RightWallRun || NewCustomMovementMode == MOVE_LeftWallRun || IsWallRunning())
		//{
		//	bChangeCamera = true;
		//}
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Returning max movement speed to normal!!"));
		//}
		CustomMovementMode = NewCustomMovementMode;
		ResetMovement();
		return true;
	}
}

void ULevelsPlayerMovementComponent::ResetMovement()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Resetting Movement")));
	//if equal to null


	if (CustomMovementMode == MOVE_CustomNone || CustomMovementMode == MOVE_Crouch)
	{
		GravityScale = DefaultGravity;
		GroundFriction= DefaultGroundFriction;
		BrakingDecelerationWalking = DefaultBrakingDeceleration;
		MaxWalkSpeed = DefaultWalkSpeed;
		MaxWalkSpeedCrouched = DefaultCrouchSpeed;

		SetPlaneConstraintEnabled(false);

		switch (CustomMovementMode) {
		case MOVE_CustomNone:
			SetMovementMode(MOVE_Walking);
			break;
		case MOVE_Slide:
			SetMovementMode(MOVE_Walking);
			break;
		case MOVE_LeftWallRun:
			SetMovementMode(MOVE_Falling);
			break;
		case MOVE_RightWallRun:
			SetMovementMode(MOVE_Falling);
			break;
		case MOVE_WallClimb:
			SetMovementMode(MOVE_Falling);
			break;
		case MOVE_LedgeGrab:
			SetMovementMode(MOVE_Falling);
			break;
		case MOVE_Mantle:
			SetMovementMode(MOVE_Walking);
			break;
		case MOVE_Sprint:
			SetMovementMode(MOVE_Walking);
			break;
		case MOVE_Crouch:
			SetMovementMode(MOVE_Walking);
			break;
		}
	}
}

void ULevelsPlayerMovementComponent::CheckQueuedMovement()
{
	if (bWantsToSlide)
	{
		SlideStart();
	}
	else if (bWantsToSprint)
	{
		SprintStart();
	}
}

void ULevelsPlayerMovementComponent::MovementCamera(float Roll)
{
	//if (bChangeCamera) {
		//tilts camera in direction of wall run or slide
		if (CustomMovementMode == MOVE_RightWallRun || IsSliding())
		{
			CameraTilt(-Roll);
		}
		else if (CustomMovementMode == MOVE_LeftWallRun)
		{
			//change the roll of the camera to give a wall run-like experience
			CameraTilt(Roll);
		}
		else
		{
			//called very often. will try to optimize later by not calling when we dont need to
			CameraTilt(0.0f);
		}
		//this was an attemp to optimize but broke the smooth camera transition with RInterpTo in camera tilt
		//bChangeCamera = false;
	//}
}


void ULevelsPlayerMovementComponent::CameraTilt(float CameraRoll)
{
	//rotates the character's camera to the Camera Rotation
	//tried to put GetWorld()->GetFirstPlayerController()->SetControlRotation (the current camera rotation) but created a bug (apparently we shouldnt get current rotation from a variable. Enjoy this long long line
	GetWorld()->GetFirstPlayerController()->SetControlRotation(FMath::RInterpTo(GetWorld()->GetFirstPlayerController()->GetControlRotation(), 
		FRotator(GetWorld()->GetFirstPlayerController()->GetControlRotation().Pitch, GetWorld()->GetFirstPlayerController()->GetControlRotation().Yaw, CameraRoll), GetWorld()->GetDeltaSeconds(), 10.f));
}

void ULevelsPlayerMovementComponent::PhysWalking(float deltaTime, int32 Iterations) 
{
	//Increases speed at the start of walking. Done by increasing acceleration when the character's velocity is below 500 (can be changed) units/s 
	//makes the game feel faster :)
	if (Velocity.Size2D() < 700.f)
	{
		//takes normal of the current accelleration vector and multiplies it by 2048
		Acceleration = Acceleration.GetSafeNormal() * 2048.0f;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

bool ULevelsPlayerMovementComponent::ForwardInput()
{
	return FVector::DotProduct(CharacterOwner->GetActorForwardVector(), GetLastInputVector()) > 0.f;
}

bool ULevelsPlayerMovementComponent::MovingForward()
{
	return (FVector::DotProduct(CharacterOwner->GetActorForwardVector(), Velocity.GetSafeNormal())) > 0;
}

void ULevelsPlayerMovementComponent::WallRunEndVectors()
{
	//get the character's current location
	CurrentLocation = CharacterOwner->GetActorLocation();
	
	//get the right vector of the character with a length of 75 for ray casting when detecting a wall for wall running
	RightDirection = CharacterOwner->GetActorRightVector()  * 75.f;
	
	//get the forward vector of the character with a length of -35 for ray casting when detecting a wall for wall running. This gives detection to the back of the character for leeway for aiming and stuff.
	ForwardDirection = CharacterOwner->GetActorForwardVector() * -35.f;
	
	//add for right endpoint
	RightEndpoint = CurrentLocation + RightDirection + ForwardDirection;

	//get the left vector of the character with a length of 75 for ray casting when detecting a wall for wall running
	LeftDirection = CharacterOwner->GetActorRightVector()  * -75.f;
	
	//add for left endpoint
	LeftEndpoint = CurrentLocation + LeftDirection + ForwardDirection;

}

bool ULevelsPlayerMovementComponent::CanWallRun()
{
	if (MovingForward() && CustomMovementMode == MOVE_CustomNone || IsWallRunning())
	{
		if (CustomMovementMode == MOVE_CustomNone || IsWallRunning())
		{
			return true;
		}
	}
	return false;
}

void ULevelsPlayerMovementComponent::WallRunUpdate()
{
	if (CanWallRun()) 
	{
		//for checking gravity scale
		//FString GravString = FString::SanitizeFloat(GravityScale);

		//get endpoints
		WallRunEndVectors();

		//boolean for whether if the velocity is fast enough for wall running
		bool PassesSpeedRequirement = Velocity.Size2D() > WallRunSpeedRequirement;

		//the character can wallrun on the right if wallrun isn't disabled, it is going fast enough, wasn't just wallrunning on the left side and has a wall detected to run on
		if (PassesSpeedRequirement && !(CustomMovementMode == MOVE_LeftWallRun) && WallRunMovement(CharacterOwner, CurrentLocation, RightEndpoint, -1.0f))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Right side wall running!!"));
			SetCustomMovementMode(MOVE_RightWallRun);
			//change gravity to give the effect that the character falls downwards as it goes along the wall
			GravityScale = FMath::FInterpTo(DefaultGravity, WallRunGravity, GetWorld()->GetDeltaSeconds(), 30.0);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, *GravString); //Check what the gravity was changed to
		}
			//same but on the left
		else if (PassesSpeedRequirement && !(CustomMovementMode == MOVE_RightWallRun) && WallRunMovement(CharacterOwner, CurrentLocation, LeftEndpoint, 1.0f))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Left side wall running!!"));
			SetCustomMovementMode(MOVE_LeftWallRun);
			GravityScale = FMath::FInterpTo(DefaultGravity, WallRunGravity, GetWorld()->GetDeltaSeconds(), 30.0);
		}
		else
		{
			//check so we don't call WallRunEnd all the time
			if (IsWallRunning())
			{
				//ends wallrunning
				WallRunEnd(WallRunCooldown);
				//test checks for gravity and WallRunSuppressed
				//GravString = FString::SanitizeFloat(GravityScale);
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("WallRunSuppressed: %s"), (bIsWallRunSuppressed ? TEXT("true") : TEXT("false"))));
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, *GravString);
			}
		}
	}
}


bool ULevelsPlayerMovementComponent::WallRunMovement(ACharacter* Character, FVector Start, FVector End, float WallRunDirection)
{
	//hit result object for ray casting
	FHitResult Hit(ForceInit);
	//FCollisionQueryParams TraceParams;
	//draws a line ingame that represent where this function is checking for wall running. Makes detection easier to understand when debugging
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 7.0f);
	//if something was hit (gets hit result if hit)
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		//store hit normal in a variable
		WallRunHitNormal = Hit.Normal;

		//For debugging. Stores the x and y of the normal vector from the wall hit.
		FString HitX = FString::SanitizeFloat(WallRunHitNormal.X);
		FString HitY = FString::SanitizeFloat(WallRunHitNormal.Y);
		//FString HitZ = FString::SanitizeFloat(WallRunHitNormal.Z);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("WallRunSuppressed: %s"), (bIsWallRunSuppressed ? TEXT("true") : TEXT("false"))));
		
		//Fixes a bug that would happen when the player is wall running and the wall ends, if the wall had corner (another wall perpendicular to it) the detection would hit the perpendicular wall as the player is flying off the first wall and cause 
		//the player to wall run on the perpendicular wall. This would cause the player to sometimes suddenly make a 90 degree turn and start wall running with the same velocity as before. The player might also just lose speed after a wall ends if the new wall run didn't continue.
		//The fix is making a variable that holds the normal vector from the previous time this functionwas called while wall running. This is initialized as a vector with all components as 0 and resets on WallRunEnd. The if statement says if the Z value from PrevWallRunHitNormal
		// is not zero (it will never be if we are wallrunning) and the difference of the current and previous hit normal is larger than the size of a vector with the components (0.5f, 0.5f, 0.5f) <- difference only made by making a 90 degree turn.  
		//If the if statement is true we call WallRunEnd and return false. tbh pretty happy I could get this to work
		if (PrevWallRunHitNormal.Z != 0 && (PrevWallRunHitNormal - WallRunHitNormal).Size2D() > FVector(0.5f, 0.5f, 0.5f).Size())
		{
			WallRunEnd(WallRunCooldown);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Wallrunning between corners BLOCKED :)")));
			return false;
		}

		//if the z component of WallRunHitNormal is between -.52 and .52 and the state is falling
		if ((WallRunHitNormal.Z < .52f && WallRunHitNormal.Z > -.52f) && IsFalling())
		{
			//cling player to wall
    		//Character->LaunchCharacter(PlayerToWallVector(WallRunHitNormal), false, false);
			//move player forward on wall
			FVector LaunchVector = FVector::CrossProduct(WallRunHitNormal, FVector(0.f, 0.f, 1.f));
			//launch player along wall to make wall running
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, *HitX);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, *HitY);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, *HitZ);
			Character->LaunchCharacter(LaunchVector * (WallRunDirection * Velocity.Size2D()), true, !WallRunGravityOn);
			//bWallRunning = true;
			PrevWallRunHitNormal = WallRunHitNormal;
			return true;
		}
		//if we are no longer falling call WallRunEnd (bug fix)
		//WallRunEnd(WallRunCooldown);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("WallRunSuppressed: %s"), (bIsWallRunSuppressed ? TEXT("true") : TEXT("false"))));
		//return false;
	}
	return false;
}

void ULevelsPlayerMovementComponent::WallRunJump()
{
	if (IsWallRunning())
	{
		WallRunEnd(WallRunJumpCooldown);
		//various algorithms for wall jumping
		//CharacterOwner->LaunchCharacter(FVector(WallRunJumpForce * WallRunHitNormal.X, WallRunJumpForce * WallRunHitNormal.Y, WallRunJumpHeight), true, true);
		//CharacterOwner->LaunchCharacter(FVector(WallRunJumpForce * Velocity.GetSafeNormal().X, WallRunJumpForce * Velocity.GetSafeNormal().Y, WallRunJumpHeight), true, true);
		//CharacterOwner->LaunchCharacter(FVector(WallRunJumpForce * WallRunHitNormal.X, Velocity.Y * WallRunHitNormal.Y, WallRunJumpHeight), true, true);
		CharacterOwner->LaunchCharacter(FVector(WallRunJumpForce * Velocity.X, WallRunJumpForce * Velocity.Y, WallRunJumpHeight), true, true);
	}
}

bool ULevelsPlayerMovementComponent::IsWallRunning()
{
	if (CustomMovementMode == MOVE_RightWallRun || CustomMovementMode == MOVE_LeftWallRun)
	{
		return true;
	}
		return false;
}

void ULevelsPlayerMovementComponent::EnableWallRun()
{
	bWallRunEnabled = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bWallRunEnabled: %s"), (bWallRunEnabled ? TEXT("true") : TEXT("false"))));
	GetWorld()->GetTimerManager().ClearTimer(WallRunCooldownTimerHandle);

}

void ULevelsPlayerMovementComponent::DisableWallRun()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Disabled Wall Run")));
	bWallRunEnabled = false;
}

void ULevelsPlayerMovementComponent::WallRunEnd(float Cooldown)
{
	SetCustomMovementMode(MOVE_CustomNone);
	//fix gravity
	PrevWallRunHitNormal = FVector::ZeroVector;
	GravityScale = DefaultGravity;
	//Set a cooldown on wallrunning
	DisableWallRun();
	GetWorld()->GetTimerManager().SetTimer(WallRunCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableWallRun, Cooldown, true);
	GetWorld()->GetTimerManager().SetTimer(QueueTimerHandle, this, &ULevelsPlayerMovementComponent::CheckQueuedMovement, Cooldown, true);
}

void ULevelsPlayerMovementComponent::WallClimbUpdate()
{
	if (CanWallClimb())
	{
		FHitResult Hit(ForceInit);
		MantleVectors();

		//EDrawDebugTrace:: for debug lines
		if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MantleEyeLevel, MantleFeetLevel, 20.f, 10.f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorArray, EDrawDebugTrace::None, Hit, true, FLinearColor::Green, FLinearColor::Red, 7.0f))
		{
			MantleTraceDistance = Hit.Distance;
			if (IsWalkable(Hit))
			{
				MantlePosition = Hit.ImpactPoint + FVector(0.f, 0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
				DisableWallClimb();
				if (SetCustomMovementMode(MOVE_LedgeGrab))
				{
					DisableMovement();
					//DisableMovement() sets the CustomMovementMode to 0, we have to change the movement mode back to LedgeGrab
					CustomMovementMode = MOVE_LedgeGrab;
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Is this a ledge grab?")));
					StopMovementImmediately();
					GravityScale = 0.f;
					GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(LedgeGrabShake);
					//UE_LOG(LogTemp, Warning, TEXT("MantleTraceDistance: %f"), MantleTraceDistance);
					//UE_LOG(LogTemp, Warning, TEXT("CapsuleHalfHeight: %f"), CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
					//check if the player can quick mantle
					if (QuickMantle())
					{
						EnableMantleCheck();
					}
					else
					{
						GetWorld()->GetTimerManager().SetTimer(MantleCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableMantleCheck, .25f, true);
					}
					//GetWorld()->GetTimerManager().SetTimer(MantleCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableMantle, 0.25f, true);
					//DisableMantle();
				}
			}
			else
			{
				WallClimbMovement();
			}
		}
		else
		{
			WallClimbMovement();
		}
	}
	else
	{
		WallClimbEnd(.35f);
	}
	
}

bool ULevelsPlayerMovementComponent::WallClimbMovement()
{
	FHitResult Hit(ForceInit);
	//DrawDebugLine(GetWorld(), MantleEyeLevel, CharacterOwner->GetActorForwardVector() * 50 + MantleFeetLevel, FColor::Green, false, 7.0f);
	if (ForwardInput() && GetWorld()->LineTraceSingleByChannel(Hit, MantleEyeLevel, CharacterOwner->GetActorForwardVector() * 50 + MantleFeetLevel, ECC_Visibility))
	{
		WallClimbHitNormal = Hit.Normal;
		SetCustomMovementMode(MOVE_WallClimb);

		CharacterOwner->LaunchCharacter(FVector(WallClimbHitNormal.X * -600.f, WallClimbHitNormal.Y * -600.f, WallClimbSpeed), true, true);
		
		return true;
	}
	else 
	{
		WallClimbEnd(.35f);
		return false;
	}
}

bool ULevelsPlayerMovementComponent::CanWallClimb()
{
	if(ForwardInput() && IsFalling() && (CustomMovementMode == MOVE_CustomNone || CustomMovementMode == MOVE_WallClimb || IsWallRunning()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Can Wall Climb")));
		return true;
	}
	return false;
}

void ULevelsPlayerMovementComponent::EnableWallClimb()
{
	bWallClimbEnabled = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bWallClimbEnabled: %s"), (bWallClimbEnabled ? TEXT("true") : TEXT("false"))));
}

void ULevelsPlayerMovementComponent::DisableWallClimb()
{
	bWallClimbEnabled = false;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bWallClimbEnabled: %s"), (bWallClimbEnabled ? TEXT("true") : TEXT("false"))));
	DisableMantle();
	//GetWorld()->GetTimerManager().SetTimer(WallClimbCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableWallClimb, Cooldown, true);
}

void ULevelsPlayerMovementComponent::WallClimbEnd(float Cooldown)
{
	if (CustomMovementMode == MOVE_LedgeGrab || CustomMovementMode == MOVE_WallClimb || CustomMovementMode == MOVE_Mantle)
	{
		if (SetCustomMovementMode(MOVE_CustomNone)) 
		{
			DisableWallClimb();
			DisableMantleCheck();
			MantleTraceDistance = 0.f;
			GetWorld()->GetTimerManager().SetTimer(WallClimbCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableWallClimb, Cooldown, false);
			GetWorld()->GetTimerManager().SetTimer(QueueTimerHandle, this, &ULevelsPlayerMovementComponent::CheckQueuedMovement, Cooldown, true);
		}
			
	}
}

bool ULevelsPlayerMovementComponent::MantleCheck()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Quick Mantle Bool %s"), (QuickMantle() ? TEXT("true") : TEXT("false"))));
	if (ForwardInput() && (CustomMovementMode == MOVE_LedgeGrab || QuickMantle()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Mantle Check passed")));
		return true;
	}
	return false;
}

bool ULevelsPlayerMovementComponent::QuickMantle()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Did a quick mantle%s"), (MantleTraceDistance > CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() ? TEXT("true") : TEXT("false"))));
	return MantleTraceDistance > CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void ULevelsPlayerMovementComponent::MantleMovement()
{
	GetWorld()->GetFirstPlayerController()->SetControlRotation(FMath::RInterpTo(GetWorld()->GetFirstPlayerController()->GetControlRotation(),
		UKismetMathLibrary::FindLookAtRotation(FVector(CharacterOwner->GetActorLocation().X, CharacterOwner->GetActorLocation().Y, 0.f), FVector(MantlePosition.X, MantlePosition.Y, 0.f)), GetWorld()->GetDeltaSeconds(), 7.f));
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MantleMovement()")));
	CharacterOwner->SetActorLocation(FMath::VInterpTo(CharacterOwner->GetActorLocation(), MantlePosition, GetWorld()->GetDeltaSeconds(), (QuickMantle() ? QuickMantleSpeed : MantleSpeed)));

	if (FVector::Distance(CharacterOwner->GetActorLocation(), MantlePosition) < 8.f)
	{
		WallClimbEnd(0.5f);
	}
}

void ULevelsPlayerMovementComponent::MantleStart()
{
	if (SetCustomMovementMode(MOVE_Mantle))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MantleStart()"), (QuickMantle() ? TEXT("true") : TEXT("false"))));
		GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(QuickMantle() ? QuickMantleShake : MantleShake);
		DisableMantleCheck();
		EnableMantle();
	}
}

void ULevelsPlayerMovementComponent::MantleVectors()
{
	GetWorld()->GetFirstPlayerController()->GetActorEyesViewPoint(MantleEyeLevel, CurrentRotation);
	MantleEyeLevel = (MantleEyeLevel + FVector(0.f, 0.f, 50.f)) + (CharacterOwner->GetActorForwardVector() * 50.f);
	MantleFeetLevel = (CharacterOwner->GetActorLocation() - (FVector(0.f, 0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - MantleHeight))) + (CharacterOwner->GetActorForwardVector() * 50.f);
}

void ULevelsPlayerMovementComponent::EnableMantle()
{
	bMantleEnabled = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bMantleEnabled: %s"), (bMantleEnabled ? TEXT("true") : TEXT("false"))));
}

void ULevelsPlayerMovementComponent::DisableMantle()
{
	bMantleEnabled = false;
	//GetWorld()->GetTimerManager().SetTimer(MantleCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableMantle, Cooldown, true);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bMantleEnabled: %s"), (bMantleEnabled ? TEXT("true") : TEXT("false"))));
}

void ULevelsPlayerMovementComponent::EnableMantleCheck()
{
	bMantleCheckEnabled = true;
}

void ULevelsPlayerMovementComponent::DisableMantleCheck()
{
	bMantleCheckEnabled = false;
}

void ULevelsPlayerMovementComponent::LedgeGrabJump()
{
	if (CustomMovementMode == MOVE_LedgeGrab || CustomMovementMode == MOVE_WallClimb || CustomMovementMode == MOVE_Mantle)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Ledge grab jump")));
		WallClimbEnd(0.35f);
		CharacterOwner->LaunchCharacter(FVector(WallClimbHitNormal.X * LedgeGrabJumpForce, WallClimbHitNormal.Y * LedgeGrabJumpForce, LedgeGrabJumpHeight), false, true);
	}
}

bool ULevelsPlayerMovementComponent::IsSliding()
{
	if (CustomMovementMode == MOVE_Slide)
	{
		return true;
	}
	return false;
}

void ULevelsPlayerMovementComponent::SlideUpdate()
{
	if (CustomMovementMode == MOVE_Slide)
	{
		if (Velocity.Size() <= 350.f)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Slide Update")));
			SlideEnd(true);
		}
	}
}

void ULevelsPlayerMovementComponent::SlideJump()
{
	if (CustomMovementMode == MOVE_Slide)
	{
		SlideEnd(false);

	}
}

void ULevelsPlayerMovementComponent::EnableSlide()
{
	bSlidingEnabled = true;
}

void ULevelsPlayerMovementComponent::DisableSlide()
{
	bSlidingEnabled = false;
}

bool ULevelsPlayerMovementComponent::CanSlide()
{
	if ((CustomMovementMode == MOVE_Sprint || bWantsToSprint) && MovingForward())
	{
		return true;
	}
	return false;
}

void ULevelsPlayerMovementComponent::SlideStart()
{
	if (CanSlide() && IsWalking())
	{
		SprintEnd();
		SetCustomMovementMode(MOVE_Slide);
		Crouch(true);
		GroundFriction = 0.f;
		BrakingDecelerationWalking = 1400.f;
		MaxWalkSpeed = 0.f;
		SetPlaneConstraintFromVectors(Velocity.GetSafeNormal(), CharacterOwner->GetActorUpVector());
		SetPlaneConstraintEnabled(true);
		FHitResult Hit(ForceInit);
		FVector SlideVector;
		DrawDebugLine(GetWorld(), CharacterOwner->GetActorLocation(), (CharacterOwner->GetActorUpVector() * -200) + CharacterOwner->GetActorLocation(), FColor::Green, false, 7.0f);
		GetWorld()->LineTraceSingleByChannel(Hit, CharacterOwner->GetActorLocation(), (CharacterOwner->GetActorUpVector() * -200) + CharacterOwner->GetActorLocation(), ECC_Visibility);
		SlideVector = FVector::CrossProduct(CharacterOwner->GetActorUpVector(), Hit.ImpactNormal) * -1.0f;
		
		//FString GravString = FString::SanitizeFloat(GravityScale);

		if (SlideVector.Z <= .02f)
		{
			FString SlideString = SlideVector.ToString();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *SlideString);
			AddImpulse(SlideImpulseForce * SlideVector, true);
		}
		EnableSlide();
		bWantsToSlide = false;
		bWantsToSprint = false;
	}
}

void ULevelsPlayerMovementComponent::SlideEnd(bool CrouchAfter)
{
	if (CustomMovementMode == MOVE_Slide)
	{
		if (CrouchAfter)
		{
			Crouch(true);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Crouch"));
			SetCustomMovementMode(MOVE_Crouch);
			MaxWalkSpeed = 300.f;
			
		}
		else
		{
			SetCustomMovementMode(MOVE_CustomNone);
			UnCrouch();
		}
		DisableSlide();
	}
}

void ULevelsPlayerMovementComponent::CrouchSlideCheck()
{
	if (CustomMovementMode == MOVE_LedgeGrab || CustomMovementMode == MOVE_WallClimb || CustomMovementMode == MOVE_Mantle)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Ledge grab jump")));
		WallClimbEnd(0.5f);
	}
	else if (IsWallRunning())
	{
		WallRunEnd(0.5f);
	}
	else if (!CanSlide())
	{
		if (CustomMovementMode == MOVE_CustomNone)
		{
			CrouchStart();
		}
		else if (CustomMovementMode == MOVE_Crouch)
		{
			CrouchEnd();
		}
	}
	else if (IsWalking())
	{
		SlideStart();
	}

	bWantsToSlide = true;
}

void ULevelsPlayerMovementComponent::CrouchJump()
{
	if (CustomMovementMode == MOVE_Crouch)
	{
		CrouchEnd();
	}
}

void ULevelsPlayerMovementComponent::CrouchStart()
{
	if (CustomMovementMode == MOVE_CustomNone)
	{
		Crouch(true);
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Crouch"));
		SetCustomMovementMode(MOVE_Crouch);
		MaxWalkSpeed = 300.f;
		bWantsToSlide = false;
		bWantsToSprint = false;
	}

}

void ULevelsPlayerMovementComponent::CrouchEnd()
{
	if (CustomMovementMode == MOVE_Crouch)
	{
		UnCrouch(true);
		SetCustomMovementMode(MOVE_CustomNone);
		bWantsToSlide = false;
		bWantsToSprint = false;
	}

}

void ULevelsPlayerMovementComponent::SprintUpdate()
{
	if (!(CustomMovementMode == MOVE_Sprint && ForwardInput()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint Update (END)"));
		SprintEnd();
	}
}

void ULevelsPlayerMovementComponent::EnableSprint()
{
	bSprintEnabled = true;
}

void ULevelsPlayerMovementComponent::DisableSprint()
{
	bSprintEnabled = false;
}

void ULevelsPlayerMovementComponent::SprintStart()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint Start"));
	CrouchEnd();
	SlideEnd(false);
	if (CustomMovementMode == MOVE_CustomNone && IsWalking()) 
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint Start2"));
		if (SetCustomMovementMode(MOVE_Sprint))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint Start3"));
			MaxWalkSpeed = SprintSpeed;
			EnableSprint();
			bWantsToSlide = false;
			bWantsToSprint = false;
		}
	}
}

void ULevelsPlayerMovementComponent::SprintEnd()
{
	if (CustomMovementMode == MOVE_Sprint)
	{
		if (SetCustomMovementMode(MOVE_CustomNone))
		{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint End!!"));
		//MaxWalkSpeed /= SprintSpeedMultiplier;
		DisableSprint();
		//GetWorld()->GetTimerManager().SetTimer(SprintCooldownTimerHandle, this, &ULevelsPlayerMovementComponent::EnableSprint, .1f, false);
		}
	}
}

void ULevelsPlayerMovementComponent::SprintJump()
{
	if (CustomMovementMode == MOVE_Sprint)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Sprint Jump"));
		SprintEnd();
		bWantsToSprint = true;
	}
}

void ULevelsPlayerMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
}


void ULevelsPlayerMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}