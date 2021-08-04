// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LevelsPlayerMovementComponent.generated.h"

class ALevels_v0Character;
class UMatineeCameraShake;

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	MOVE_CustomNone = 0,
	MOVE_Slide = 1,
	MOVE_LeftWallRun = 2,
	MOVE_RightWallRun = 3,
	MOVE_WallClimb = 4,
	MOVE_LedgeGrab = 5,
	MOVE_Mantle = 6,
	MOVE_Sprint = 7,
	MOVE_Crouch = 8
};

UCLASS()
class LEVELS_V0_API ULevelsPlayerMovementComponent : public UCharacterMovementComponent
{

	GENERATED_BODY()

public:

	//Overrides

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	//virtual bool DoJump(bool bReplayingMoves) override;



#pragma region Movement Mode Implementations

	void PhysSprint(float deltaTime, int32 Iterations);
	void PhysSlide(float deltaTime, int32 Iterations);

#pragma endregion	



protected:

	//for wall running
	FVector RightDirection;
	FVector LeftDirection;
	FVector CurrentLocation;
	FVector ForwardDirection;
	FVector RightEndpoint;
	FVector LeftEndpoint;
	FVector WallRunHitNormal;
	FVector WallClimbHitNormal;
	FVector PrevWallRunHitNormal;

	//for mantling
	FVector MantleEyeLevel;
	FVector MantleFeetLevel;
	FVector MantleHitNormal;
	FVector MantlePosition;
	float MantleTraceDistance;
	
	//booleans for states (movement modes)
	bool bWallRunEnabled = false;
	bool bWallClimbEnabled = false;
	bool bMantleEnabled = false;
	bool bMantleCheckEnabled = false;
	bool bSprintEnabled = false;
	bool bCrouchEnabled = false;
	bool bChangeCamera = false;
	bool bWantsToSprint = false;
	bool bWantsToSlide = false;
	bool bSlidingEnabled = false;

	//timer handles
	FTimerHandle WallRunCooldownTimerHandle;
	FTimerHandle WallRunTimerHandle;
	FTimerHandle CameraTimerHandle;
	FTimerHandle WallClimbCooldownTimerHandle;
	FTimerHandle MantleCooldownTimerHandle;
	FTimerHandle SprintCooldownTimerHandle;
	FTimerHandle QueueTimerHandle;
	FRotator CurrentRotation;
	TArray<AActor*, FDefaultAllocator> ActorArray;

	//set default gravity scale value to a variable
	float DefaultGravity;
	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;
	float DefaultWalkSpeed;
	float DefaultCrouchSpeed;

#pragma endregion

public:


	ULevelsPlayerMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Changes current custom movement */
	UFUNCTION()
		bool SetCustomMovementMode(uint8 NewCustomMovementMode);

	/** Resets movement state based on the current movement state */
	UFUNCTION()
		void ResetMovement();

	/** Resets movement state based on the current movement state */
	UFUNCTION()
		void CheckQueuedMovement();

	//the roll of the camera when wall jumping
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float MovementCameraRoll = 15.f;

	/** Checks if the camera is correct based on the movement state */
	UFUNCTION()
		void MovementCamera(float Roll);

	/** Tilts camera */
	UFUNCTION()
		void CameraTilt(float CameraRoll);

	//Listens and checks for when to change movement modes and calls the movement mode functions
	UFUNCTION()
		void WallMovementCheck();

	//gets if the player is inputting forward
	UFUNCTION()
		bool ForwardInput();

	//Returns if the player's forward velocity is positive
	UFUNCTION()
		bool MovingForward();

	/** Get endpoints for character in prep for a raycast when checking if there is a wall to wall run on */
	UFUNCTION()
		void WallRunEndVectors();

	/** Check for and perform wallruns */
	UFUNCTION()
		void WallRunUpdate();

	/** Additional checks for if the player should be able to wall run */
	UFUNCTION()
		bool CanWallRun();

	/** Does a raycast and performs the required movement to wall run */
	UFUNCTION()
		bool WallRunMovement(ACharacter* Character, FVector Start, FVector End, float WallRunDirection);

	/** Wall jump */
	UFUNCTION()
		void WallRunJump();

	/** End wall run state */
	UFUNCTION()
		void WallRunEnd(float Cooldown);

	//the gravity of the player when wall running
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunGravity = .10f;

	//the cooldown time for wallrunning 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunCooldown = .75f;

	//the cooldown time for wallrunning after wall jumping
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunJumpCooldown = .25f;

	//height gained from a wall jump
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunJumpHeight = 400.f;

	//right now this multiplies the velocity the player is going when performing a wall jump
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunJumpForce = 1.1f;

	//Speed the player must go in order to wall run. 0 for no requirement
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		float WallRunSpeedRequirement;

	/** Enables wall run and resets timer */
	UFUNCTION()
		void EnableWallRun();

	/** Disables wall run */
	UFUNCTION()
		void DisableWallRun();

	/** If true use Gravity on walls */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Run")
		bool WallRunGravityOn = false;

	/** Returns if player is wall running */
	UFUNCTION()
		bool IsWallRunning();

	/** Check for and perform wall climbing */
	UFUNCTION()
		void WallClimbUpdate();

	/** Does a raycast and gets required movement to wall climb */
	UFUNCTION()
		bool WallClimbMovement();

	/** Check if player can perform a wall Climb */
	UFUNCTION()
		bool CanWallClimb();

	//speed the player climbs walls
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Climb")
		float WallClimbSpeed = 400.f;

	/** Enables wall climb and resets timer  */
	UFUNCTION()
		void EnableWallClimb();

	/** Disables wall climb */
	UFUNCTION()
		void DisableWallClimb();

	/** End wall climb state */
	UFUNCTION()
		void WallClimbEnd(float Cooldown);

	/** Check if player can mantle */
	UFUNCTION()
		bool MantleCheck();

	/** Check for is player can do a quick mantle */
	UFUNCTION()
		bool QuickMantle();

	//speed the player mantles
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mantle")
		float MantleSpeed = 10.f;

	//speed the player quick mantles
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mantle")
		float QuickMantleSpeed = 20.f;

	/** Performs the movement for a mantle */
	UFUNCTION()
		void MantleMovement();

	/** Check for and enable mantling */
	UFUNCTION()
		void MantleStart();

	/** Gets the vectors needed for checking for mantling */
	UFUNCTION()
		void MantleVectors();

	//how tall something must be to mantle onto it
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mantle")
	float MantleHeight = 40.f;

	/** Enables mantle  */
	UFUNCTION()
		void EnableMantle();

	/** Disables mantle */
	UFUNCTION()
		void DisableMantle();

	/** Enables mantle check  */
	UFUNCTION()
		void EnableMantleCheck();

	/** Disables mantle check */
	UFUNCTION()
		void DisableMantleCheck();

	/** Jumps of a wall while grabbing on a ledge */
	UFUNCTION()
		void LedgeGrabJump();

	//The force of the jump off a ledge
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Climb")
		float LedgeGrabJumpForce = 300.f;

	//The height of the jump off of a ledge
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wall Climb")
		float LedgeGrabJumpHeight = 400.f;

	/** Returns if player is sliding */
	UFUNCTION()
		bool IsSliding();

	/** Checks if the player is going fast enough to slide while sliding, if not it puts the player in a crouch */
	UFUNCTION()
		void SlideUpdate();

	/** Jump in sliding state */
	UFUNCTION()
		void SlideJump();
	
	/** Enable slide check */
	UFUNCTION()
		void EnableSlide();

	/** Disable slide check */
	UFUNCTION()
		void DisableSlide();

	/** Start slide state */
	UFUNCTION()
		void SlideStart();

	/** End slide state */
	UFUNCTION()
		void SlideEnd(bool Crouch);

	/** Start crouch state */
	UFUNCTION()
		void CrouchSlideCheck();

	/** Jump in crouching state */
	UFUNCTION()
		void CrouchJump();

	/** Start crouch state */
	UFUNCTION()
		void CrouchStart();

	/** End crouch state */
	UFUNCTION()
		void CrouchEnd();

	/** Checks if player can slide */
		UFUNCTION()
		bool CanSlide();

	//The amount of impule the player recieves while sliding
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Slide")
		float SlideImpulseForce = 600.f;

	/** Checks if the player is pressing W while sprinting, if not stop sprinting */
	UFUNCTION()
		void SprintUpdate();

	/** Disable sprint update */
	UFUNCTION()
		void EnableSprint();

	/** Disable sprint update */
	UFUNCTION()
		void DisableSprint();

	/** Start sprint state */
	UFUNCTION()
		void SprintStart();

	/** End sprint state */
	UFUNCTION()
		void SprintEnd();

	/** Handles if the player will continue sprinting after a jump and land */
	UFUNCTION()
		void SprintJump();

	/** Gets called whenever the player presses space */
	UFUNCTION()
		void OnJump();


	//Camera shakes

	UPROPERTY(EditAnywhere, Category = "Camera Shakes")
		TSubclassOf<UMatineeCameraShake> LedgeGrabShake;

	UPROPERTY(EditAnywhere, Category = "Camera Shakes")
		TSubclassOf<UMatineeCameraShake> JumpLandShake;

	UPROPERTY(EditAnywhere, Category = "Camera Shakes")
		TSubclassOf<UMatineeCameraShake> MantleShake;

	UPROPERTY(EditAnywhere, Category = "Camera Shakes")
		TSubclassOf<UMatineeCameraShake> QuickMantleShake;
 
	//Sprinting speed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sprint")
		float SprintSpeed = 1500.f;
};

