// Fill out your copyright notice in the Description page of Project Settings.


#include "DropGames_PlayerController.h"
#include "DropGames_TestCharacter.h"

void ADropGames_PlayerController::ClientRestart_Implementation(class APawn* NewPawn)
{
    ADropGames_TestCharacter* Character = Cast<ADropGames_TestCharacter>(NewPawn);
    UE_LOG(LogTemp, Warning, TEXT("HERE"));
    if(Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("HERE2"));

        Character -> RefreshAbilities();
    }
}
