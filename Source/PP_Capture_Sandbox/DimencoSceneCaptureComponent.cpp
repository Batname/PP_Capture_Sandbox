// Fill out your copyright notice in the Description page of Project Settings.

#include "DimencoSceneCaptureComponent.h"
#include "MyGameViewportClient.h"


#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Runtime/Engine/Public/UnrealEngine.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"

#include "Runtime/Core/Public/Modules/ModuleManager.h"

#include "SlateBasics.h"
#include "SlateApplication.h"
#include "PipelineStateCache.h"

#include "ScreenRendering.h"
#include "RenderCore.h"
#include "RHIStaticStates.h"
#include "RendererInterface.h"

#include "Runtime/Engine/Public/Slate/SceneViewport.h"


// Sets default values for this component's properties
UDimencoSceneCaptureComponent::UDimencoSceneCaptureComponent(FVTableHelper& Helper)
	: Super(Helper)
	, ImageWrapperModule(FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))
{
}



// Sets default values for this component's properties
UDimencoSceneCaptureComponent::UDimencoSceneCaptureComponent()
	: ImageWrapperModule(FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDimencoSceneCaptureComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set reference to player controller
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Register slate rendered delegate
	FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();//.Get();
	SlateRenderer->OnSlateWindowRendered().RemoveAll(this);
	SlateRenderer->OnSlateWindowRendered().AddUObject(this, &UDimencoSceneCaptureComponent::OnSlateRendered);
	
}


// Called every frame
void UDimencoSceneCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update Viewport geometry
	if (PlayerController && PlayerController->GetLocalPlayer())
	{
		UGameViewportClient* GameViewportClient = PlayerController->GetLocalPlayer()->ViewportClient;
		FGeometry ViewportGeometry;
		const bool bResult = FindViewportGeometry(GameViewportClient->GetWindow(), ViewportGeometry);
		if (bResult)
		{
			ViewportPositionGeometry = ViewportGeometry.LocalToAbsolute(FVector2D::ZeroVector);
			ViewportSizeGeometry = ViewportGeometry.GetLocalSize();
		}
	}
}

bool UDimencoSceneCaptureComponent::FindViewportGeometry(TSharedPtr<SWindow> WindowWidget, FGeometry & OutGeometry) const
{
	if (WindowWidget.IsValid())
	{
		return FindViewportGeometryInternal(WindowWidget->GetWindowGeometryInWindow(), WindowWidget, OutGeometry);
	}

	return false;
}

bool UDimencoSceneCaptureComponent::FindViewportGeometryInternal(const FGeometry & Geometry, TSharedPtr<SWidget> Widget, FGeometry & OutGeometry) const
{
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	Widget->ArrangeChildren(Geometry, ArrangedChildren);
	for (int32 Index = 0; Index < ArrangedChildren.Num(); ++Index)
	{
		TSharedPtr<SWidget> ChildWidget = ArrangedChildren[Index].Widget;
		FGeometry ChildGeometry = ArrangedChildren[Index].Geometry;

		//@todo: Don't understand why casting not working??? It's always return true .IsValid()
		//TSharedPtr<SViewport> Viewport = StaticCastSharedPtr<SViewport>(ChildWidget);
		// !!! OK !!! I know now why it is not working. We need dynamic cast. My Bad :{
		static FName NAME_Viewport(TEXT("SGameLayerManager"));
		if (ChildWidget->GetType() == NAME_Viewport)
		{
			OutGeometry = ArrangedChildren[Index].Geometry;
			return true;
		}
		else
		{
			const bool bResult = FindViewportGeometryInternal(ChildGeometry, ChildWidget, OutGeometry);
			if (bResult)
			{
				return true;
			}
		}
	}

	return false;
}

void UDimencoSceneCaptureComponent::OnSlateRendered(SWindow & SlateWindow, void * ViewportRHIPtr)
{
	// for now only once
	if (!DoScreenShot)
	{
		return;
	}

	if (GEngine == nullptr || GEngine->GameViewport == nullptr || GWorld == nullptr)
	{
		return;
	}

	PlayerController->SetPause(true);

	if (!SlateWindow.IsFocusedInitially())
	{
		return;
	}

	const FViewportRHIRef* ViewportRHI = (const FViewportRHIRef*)ViewportRHIPtr;
	static const FName RendererModuleName("Renderer");
	IRendererModule& RendererModule = FModuleManager::GetModuleChecked<IRendererModule>(RendererModuleName);

	UGameViewportClient* GameViewportClient = GEngine->GameViewport;
	FVector2D WindowSize = GameViewportClient->GetWindow()->GetSizeInScreen();
	FVector2D UV = ViewportPositionGeometry / WindowSize;
	FVector2D UVSize = ViewportSizeGeometry / WindowSize;

	struct FCopyVideoFrame
	{
		FViewportRHIRef ViewportRHI;
		IRendererModule* RendererModule;
		FIntPoint Resolution;
		FVector2D UV;
		FVector2D UVSize;
	};

	FCopyVideoFrame CopyVideoFrame =
	{
		*ViewportRHI,
		&RendererModule,
		FIntPoint(FrameWidth, FrameHeight),
		UV,
		UVSize,
	};

	FCopyVideoFrame Context = CopyVideoFrame;

	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[&, Context](FRHICommandListImmediate& RHICmdList)
	{
		FPooledRenderTargetDesc OutputDesc(FPooledRenderTargetDesc::Create2DDesc(Context.Resolution, PF_A16B16G16R16, FClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable, false));

		const auto FeatureLevel = GMaxRHIFeatureLevel;

		TRefCountPtr<IPooledRenderTarget> ResampleTexturePooledRenderTarget;
		Context.RendererModule->RenderTargetPoolFindFreeElement(RHICmdList, OutputDesc, ResampleTexturePooledRenderTarget, TEXT("RemoteControlTexture"));
		check(ResampleTexturePooledRenderTarget);

		const FSceneRenderTargetItem& DestRenderTarget = ResampleTexturePooledRenderTarget->GetRenderTargetItem();

		SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());
		RHICmdList.SetViewport(0, 0, 0.0f, Context.Resolution.X, Context.Resolution.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

		// @todo livestream: Ideally this "desktop background color" should be configurable in the editor's preferences
		//RHICmdList.Clear(true, FLinearColor(0.02f, 0.02f, 0.2f), false, 0.f, false, 0x00, FIntRect());

		FTexture2DRHIRef ViewportBackBuffer = RHICmdList.GetViewportBackBuffer(Context.ViewportRHI);

		auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = Context.RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		//SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		RHICmdList.SetGraphicsPipelineState(GetAndOrCreateGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::CheckApply));

		// Drawing 1:1, so no filtering needed
		PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), ViewportBackBuffer);

		Context.RendererModule->DrawRectangle(
			RHICmdList,
			0, 0,		// Dest X, Y
			Context.Resolution.X, Context.Resolution.Y,	// Dest Width, Height
			Context.UV.X, Context.UV.Y,		// Source U, V
			Context.UVSize.X, Context.UVSize.Y,		// Source USize, VSize
			Context.Resolution,		// Target buffer size
			FIntPoint(1, 1),		// Source texture size
			*VertexShader,
			EDRF_Default);

		FIntRect Rect = FIntRect(0, 0, Context.Resolution.X, Context.Resolution.Y);

		TArray<FColor> OutData;
		RHICmdList.ReadSurfaceData(
			DestRenderTarget.TargetableTexture,
			Rect,
			OutData,
			FReadSurfaceDataFlags()
		);

		UE_LOG(LogTemp, Warning, TEXT("OutData[156] %s"), *OutData[156].ToString());

		// BGRA to RGBA
		for (int32 Index = 0; Index < OutData.Num(); Index++)
		{
			auto Tmp = OutData[Index].B;
			OutData[Index].B = OutData[Index].R;
			OutData[Index].R = Tmp;
			OutData[Index].A = 225;
		}

		SaveCaptureComponent(OutData, 1, TEXT("Frame"), EImageFormat::PNG, FrameWidth, FrameHeight);
	});


	// Set false after execution
	UE_LOG(LogTemp, Warning, TEXT("DoScreenShot"));
	DoScreenShot = false;
}

void UDimencoSceneCaptureComponent::TakeScreenShot()
{
	UE_LOG(LogTemp, Warning, TEXT("TakeScreenShot"));

	UMyGameViewportClient* MyGameViewportClient = Cast<UMyGameViewportClient>(GEngine->GameViewport);
	UE_LOG(LogTemp, Warning, TEXT("MyGameViewportClient %p"), MyGameViewportClient);


	FViewport* Viewport = GEngine->GameViewport->Viewport;

	FVector		PlayerLocation;
	FRotator	PlayerRotation;
	PlayerController->GetPlayerViewPoint(PlayerLocation, PlayerRotation);
	UE_LOG(LogTemp, Warning, TEXT("PlayerLocation %s"), *PlayerLocation.ToString());



	PlayerController->SetPause(true);
	PlayerController->ConsoleCommand(TEXT("HighResShot 3840x2160"), true);

	UMyGameViewportClient::OnDimencoScreenshotCaptured().AddUObject(this, &UDimencoSceneCaptureComponent::OnDimencoScreenshotCaptured);
}

void UDimencoSceneCaptureComponent::SaveCaptureComponent(const TArray<FColor>& SeparateImg, int32 Index, const FString & Name, EImageFormat Format, float Width, float Height)
{
	// Generate name
	FString Timestamp = FString::Printf(TEXT("%s"), *FDateTime::Now().ToString());
	FString FrameString = FString::Printf(TEXT("%s.png"), *Name);
	FString OutputDir = FPaths::ProjectSavedDir() / TEXT("Dimenco");
	FString Filename = OutputDir / Timestamp / FrameString;

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

	ImageWrapper->SetRaw(SeparateImg.GetData(), SeparateImg.GetAllocatedSize(), Width, Height, ERGBFormat::RGBA, 8);
	const TArray<uint8>& CompressedImageData = ImageWrapper->GetCompressed(100);
	FFileHelper::SaveArrayToFile(CompressedImageData, *Filename);
}

void UDimencoSceneCaptureComponent::OnDimencoScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Colors)
{
	UE_LOG(LogTemp, Warning, TEXT("UDimencoSceneCaptureComponent::OnDimencoScreenshotCaptured"));
}

