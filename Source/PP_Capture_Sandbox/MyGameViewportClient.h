// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "MyGameViewportClient.generated.h"

//DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDimencoScreenshotCaptured, int32 /*Width*/, int32 /*Height*/, const TArray<FColor>& /*Colors*/);

DECLARE_MULTICAST_DELEGATE(FOnDimencoScreenshotCaptured);
/**
 * 
 */
UCLASS(BlueprintType)
class PP_CAPTURE_SANDBOX_API UMyGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	UMyGameViewportClient(const FObjectInitializer & ObjectInitializer);

	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;
	virtual void ProcessScreenShots(FViewport* InViewport) override;


	/** Accessor for delegate called when a screenshot is captured */
	static FOnDimencoScreenshotCaptured& OnDimencoScreenshotCaptured()
	{
		return DimencoScreenshotCapturedDelegate;
	}
	
private:
	FMatrix OffAxisMatrix;
	FVector CameraPosition;
	FRotator CamaraRotation;

	FName CurrentBufferVisualizationMode;

	/** Delegate called at the end of the frame when a screenshot is captured */
	static FOnDimencoScreenshotCaptured DimencoScreenshotCapturedDelegate;

	int FrameDelay = 10;
	int FrameDelayCounter = 0;
	bool bIsCounting = false;

public:
	FVector ScreenshotViewLocation = FVector::ZeroVector;

	int ScreenShotCounter = 0;

	FString ScreenShotTimestampFolder = "Win";
};
