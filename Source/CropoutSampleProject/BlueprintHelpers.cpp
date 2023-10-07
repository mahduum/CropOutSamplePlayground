// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintHelpers.h"

#include "Kismet/KismetSystemLibrary.h"
#include "UObject/SavePackage.h"

bool UBlueprintHelpers::SaveObjectToPackage(UObject* ObjectToSave, const FString PackageName)
{
	FPackageName::RegisterMountPoint("/MyMountPoint/", FPaths::ProjectPluginsDir());
	// auto ContentDir = FPaths::ProjectContentDir();
	auto PackagePath = FString::Printf(TEXT("%s%s"), *FPaths::ProjectPluginsDir(), *PackageName);
	UPackage* Package = CreatePackage(*PackagePath);
	
	// const FString DirName = FPackageName::GetLongPackagePath(Package->GetFullName());
	// const FString FileName = FPaths::GetBaseFilename(Package->GetName());
	// const FString SavePath = FString::Printf(TEXT("%s/%s"), *DirName, *FileName);//todo add third param as some id

	// Set the outer of the object to the package
	ObjectToSave->Rename(nullptr, Package);

	const FString SavePath = FString::Printf(TEXT("%s/%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
	FSavePackageArgs Args;
	// Save the package to disk
	UE_LOG(LogCore, Display, TEXT("Saving object with package: %s, under path: %s"), *Package->GetName(), *PackagePath);
	return UPackage::SavePackage(Package, ObjectToSave, *PackagePath, FSavePackageArgs());
}

UObject* UBlueprintHelpers::LoadObjectFromPackage(const FString InLongPackageName, const FString InLongFileName)
{
	//todo make async
	auto ContentDir = FPaths::ProjectContentDir();
	auto PackagePath = FString::Printf(TEXT("%s%s"), *ContentDir, *InLongPackageName);
	UPackage* Package = LoadPackage(nullptr, *PackagePath, LOAD_None);
	UObject* Object = LoadObject<UObject>(Package, *InLongFileName);
	return Object;
}

UPackage* UBlueprintHelpers::LoadPackageLocal(const FString InLongPackageName)
{
	auto ContentDir = FPaths::ProjectContentDir();
	auto PackagePath = FString::Printf(TEXT("%s%s"), *ContentDir, *InLongPackageName);
	return LoadPackage(nullptr, *PackagePath, LOAD_None);
}
