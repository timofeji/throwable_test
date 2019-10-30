// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DropGames_TestCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interfaces/PickUpable.h"
#include "Pickups/Throwable.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ADropGames_TestCharacter

ADropGames_TestCharacter::ADropGames_TestCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    
    // set our turn rates for input
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;
    
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    
    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
    
    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
    
    // Create a camera boom (pulls in towards the player if there is a collision)
    ThrowableModeBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThrowableModeBoom"));
    ThrowableModeBoom->SetupAttachment(RootComponent);
    ThrowableModeBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character
    ThrowableModeBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
    
    // Create a follow camera
    ThrowableModeCam = CreateDefaultSubobject<UCameraComponent>(TEXT("ThrowableModeCam"));
    ThrowableModeCam->SetupAttachment(ThrowableModeBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    ThrowableModeCam->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
    
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADropGames_TestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Set up gameplay key bindings
    check(PlayerInputComponent);
    PlayerInputComponent->BindAction("PickItem", IE_Pressed, this, &ADropGames_TestCharacter::PickItemServer);
    PlayerInputComponent->BindAction("ThrowItem", IE_Pressed, this, &ADropGames_TestCharacter::ActivateThrowAbilityServer);

    

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    PlayerInputComponent->BindAxis("MoveForward", this, &ADropGames_TestCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ADropGames_TestCharacter::MoveRight);
    
    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &ADropGames_TestCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ADropGames_TestCharacter::LookUpAtRate);
    
    // handle touch devices
    PlayerInputComponent->BindTouch(IE_Pressed, this, &ADropGames_TestCharacter::TouchStarted);
    PlayerInputComponent->BindTouch(IE_Released, this, &ADropGames_TestCharacter::TouchStopped);
    
    // VR headset functionality
    PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADropGames_TestCharacter::OnResetVR);
}

void ADropGames_TestCharacter::BeginPlay()
{
    Super::BeginPlay();
    if(AbilitySystem)
    {
        if (HasAuthority() && ThrowAbility)
        {
            AbilitySystem->GiveAbility(FGameplayAbilitySpec(ThrowAbility.GetDefaultObject(), 1, 0));
        }
        AbilitySystem->InitAbilityActorInfo(this, this);
    }
}

void ADropGames_TestCharacter::PossessedBy(AController * NewController)
{
    Super::PossessedBy(NewController);
}

void ADropGames_TestCharacter::OnResetVR()
{
    UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADropGames_TestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
    Jump();
}

void ADropGames_TestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
    StopJumping();
}

void ADropGames_TestCharacter::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADropGames_TestCharacter::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADropGames_TestCharacter::MoveForward(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f))
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ADropGames_TestCharacter::MoveRight(float Value)
{
    if ( (Controller != NULL) && (Value != 0.0f) )
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        // add movement in that direction
        AddMovementInput(Direction, Value);
    }
}







/*Test Implementation*/
void ADropGames_TestCharacter::Tick(float DeltaTime)
{
    //check for pickupable
    CheckForPickUpableInView();
}
void ADropGames_TestCharacter::CheckForPickUpableInView()
{
    AActor* PickUpable = GetPickUpableInView();
    if(PickUpable)
    {
        FocusedPickUpable = PickUpable;
    }
}
AActor* ADropGames_TestCharacter::GetPickUpableInView()
{
    FVector CamLoc;
    FRotator CamRot;
    
    if (Controller == nullptr || PickedUpItem)
    {
        return nullptr;
    }

    Controller->GetPlayerViewPoint(CamLoc, CamRot);
    const FVector TraceStart = CamLoc;
    const FVector Direction = CamRot.Vector();
    const FVector TraceEnd = TraceStart + (Direction * FocusDistance);
   
    FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
    TraceParams.bReturnPhysicalMaterial = false;
    TraceParams.bTraceComplex = false;
    
    FHitResult Hit(ForceInit);
    
    GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

    AActor* HitActor = Hit.GetActor();
    if(HitActor)
    {
        bool HasInterface = HitActor->GetClass()->ImplementsInterface(UPickUpable::StaticClass());
        if(HasInterface)
        {
            float Distance = FVector::Dist(GetActorLocation(), HitActor -> GetActorLocation());
            
            if(Distance <= PickUpDistance)
            {
                OnItemCanBePicked();
            }
            else
            {
                OnItemCantBePicked();
            }
            return HitActor;
        }
        else
        {
            if(!PickedUpItem)
            {
                 OnItemCantBePicked();
            }
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}
void ADropGames_TestCharacter::EnterThrowableCamMode()
{
    FollowCamera -> Deactivate();
    ThrowableModeCam -> Activate();
}
void ADropGames_TestCharacter::EnterDefaultCamMode()
{
    FollowCamera -> Activate();
    ThrowableModeCam -> Deactivate();
}

void ADropGames_TestCharacter::PickItemServer_Implementation()
{
    if(HasAuthority())
    {
        PickItemMulticast();
    }
}
void ADropGames_TestCharacter::PickItemMulticast_Implementation()
{
    if(FocusedPickUpable)
    {
        float Distance = FVector::Dist(GetActorLocation(), FocusedPickUpable -> GetActorLocation());
        if(Distance <= PickUpDistance)
        {
            PickedUpItem = FocusedPickUpable;
            FocusedPickUpable = nullptr;
            
            if (PickedUpItem->GetClass()->ImplementsInterface(UPickUpable::StaticClass()))
            {
                IPickUpable::Execute_Pickup(PickedUpItem);
            }
            PlayAnimMontage(PickItemAnimation);
            OnItemCantBePicked();
        }
    }
}

void ADropGames_TestCharacter::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADropGames_TestCharacter, PickedUpItem);
    DOREPLIFETIME(ADropGames_TestCharacter, FocusedPickUpable);
}

void ADropGames_TestCharacter::ActivateThrowAbilityServer_Implementation()
{
    if(HasAuthority())
    {
        RefreshAbilities();
        AbilitySystem -> TryActivateAbilityByClass(ThrowAbility, true);
        ActivateThrowAbilityMulticast();
    }
}
void ADropGames_TestCharacter::ActivateThrowAbilityMulticast_Implementation()
{
    if(PickedUpItem)
    {
        PlayAnimMontage(ThrowItemAnimation);
    }
}

bool ADropGames_TestCharacter::HasThrowableInHand() const
{
    //check for pickupable
    return PickedUpItem != nullptr;
}
void ADropGames_TestCharacter::AddHandAttachment(AActor* Attachment)
{
    FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
    Attachment -> AttachToComponent(GetMesh(), Rules, TEXT("hand_rSocket"));
}
void ADropGames_TestCharacter::ThrowItem()
{
    if(PickedUpItem)
    {
        AThrowable* Throwable = Cast<AThrowable>(PickedUpItem);
        if(Throwable)
        {
            EnterDefaultCamMode();
            PickedUpItem = nullptr;
            Throwable -> Throw(this);
        }
    }
}
void ADropGames_TestCharacter::RefreshAbilities()
{
    AbilitySystem->RefreshAbilityActorInfo();
}
