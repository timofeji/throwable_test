// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DropGames_TestCharacter.generated.h"

UCLASS(config=Game)
class ADropGames_TestCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
    
    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    
    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;
    
    UAbilitySystemComponent* GetAbilitySystemComponent() const override //We add this function, overriding it from IAbilitySystemInterface.
    {
        return AbilitySystem;
    };
    
public:
    ADropGames_TestCharacter();
    
    virtual void PossessedBy(class AController * NewController) override;
    virtual void BeginPlay() override;
    
    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float BaseTurnRate;
    
    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float BaseLookUpRate;
    
    
protected:
    
    /** Resets HMD orientation in VR. */
    void OnResetVR();
    
    /** Called for forwards/backward input */
    void MoveForward(float Value);
    
    /** Called for side to side input */
    void MoveRight(float Value);
    
    /**
     * Called via input to turn at a given rate.
     * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    void TurnAtRate(float Rate);
    
    /**
     * Called via input to turn look up/down at a given rate.
     * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    void LookUpAtRate(float Rate);
    
    /** Handler for when a touch input begins. */
    void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
    
    /** Handler for when a touch input stops. */
    void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
    
protected:
    // APawn interface
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    // End of APawn interface
    
public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    
    
    
    
    
    
    /*FOCUS FUNCTIONALITY**/
private:
    UPROPERTY(Replicated)
    AActor* FocusedPickUpable;
    
    void CheckForPickUpableInView();
    AActor* GetPickUpableInView();
    
    UFUNCTION(BlueprintCallable)
    void EnterThrowableCamMode();
    UFUNCTION(BlueprintCallable)
    void EnterDefaultCamMode();
    
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
    class UAbilitySystemComponent* AbilitySystem;
    
    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* PickItemAnimation;
    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* ThrowItemAnimation;
    
    UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly)
    AActor* PickedUpItem;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsThrowing;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsPicking;
    
    UPROPERTY(EditAnywhere)
    float PickUpDistance;
    UPROPERTY(EditAnywhere)
    float FocusDistance;
    
    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* ThrowableModeBoom;
    
    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* ThrowableModeCam;
    
    virtual void Tick(float DeltaTime) override;
    
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnItemCanBePicked();
    UFUNCTION(BlueprintImplementableEvent)
    void OnItemCantBePicked();
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void PickItemServer();
    UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
    void PickItemMulticast();
    
    /*THROWING FUNCTIONALITY**/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UGameplayAbility> ThrowAbility;
    
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void ActivateThrowAbilityServer();
    UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
    void ActivateThrowAbilityMulticast();
    
    UFUNCTION(BlueprintCallable)
    bool HasThrowableInHand() const;
    
    UFUNCTION(BlueprintCallable)
    void AddHandAttachment(AActor* Attachment);
    UFUNCTION(BlueprintCallable)
    void ThrowItem();
    
    UFUNCTION(BlueprintCallable)
    void RefreshAbilities();
};

