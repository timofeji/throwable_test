// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PickUpable.h"
#include "Throwable.generated.h"

UCLASS()
class DROPGAMES_TEST_API AThrowable : public AActor, public IPickUpable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Throwable")
    void Throw(AActor* InstigatorActor);

    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ThrowSpeed;

};
