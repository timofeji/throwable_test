// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DropGames_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DROPGAMES_TEST_API ADropGames_PlayerController : public APlayerController
{
	GENERATED_BODY()
    
    virtual void ClientRestart_Implementation(class APawn* NewPawn) override;

	
};
