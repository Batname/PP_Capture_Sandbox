// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Runtime/SlateCore/Public/Layout/Geometry.h"
#include "IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "DimencoSceneCaptureComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PP_CAPTURE_SANDBOX_API UDimencoSceneCaptureComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDimencoSceneCaptureComponent();

	UDimencoSceneCaptureComponent(FVTableHelper& Helper);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Copy from frame buffer  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimenco")
	int32 FrameWidth = 2560;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimenco")
	int32 FrameHeight = 1440;

	bool bIsBufferReady = false;

	FVector2D ViewportPositionGeometry = FVector2D::ZeroVector;
	FVector2D ViewportSizeGeometry = FVector2D::ZeroVector;

	bool FindViewportGeometry(TSharedPtr<SWindow> WindowWidget, FGeometry& OutGeometry) const;
	bool FindViewportGeometryInternal(const FGeometry& Geometry, TSharedPtr<SWidget> Widget, FGeometry& OutGeometry) const;

	void OnSlateRendered(class SWindow& SlateWindow, void* ViewportRHIPtr);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Miscellaneous ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
private:
	class APlayerController* PlayerController = nullptr;
	class AGameModeBase* GameMode;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimenco")
	bool DoScreenShot = false;

	UFUNCTION(BlueprintCallable, Category = "Dimenco")
	void TakeScreenShot();
	
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Save capture ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	void SaveCaptureComponent(const TArray<FColor>& SeparateImg, int32 Index, const FString& Name, EImageFormat Format, int32 Width, int32 Height);

private:
	IImageWrapperModule& ImageWrapperModule;

	class UMyGameViewportClient* MyGameViewportClient;
	class FViewport* Viewport;

	void OnDimencoScreenshotCaptured();

	int ScreenShotCounter = 0;
	bool bIsRunning = false;
};
