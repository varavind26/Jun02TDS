// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "JunTDS/FuncLibrary/Types.h"
#include "Engine/DataTable.h"
#include "JunTDS/WeaponDefault.h"
#include "JunTDSGameInstance.generated.h"

UCLASS()
class JUNTDS_API UJunTDSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " WeaponSetting ")
		UDataTable* WeaponInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
		bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
};