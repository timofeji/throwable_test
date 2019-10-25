// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickUpable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickUpable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DROPGAMES_TEST_API IPickUpable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "PickUpable")
    void Pickup();
};
