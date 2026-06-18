// Copyright Plot. SDK code: MIT.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlotRoom.h"
#include "PlotClient.generated.h"

/** Result delegate for UPlotClient::Join. On success `bSuccess` is true and
 *  `Room` is a live connection; on failure `Error` describes what went wrong. */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FPlotJoinResult, bool, bSuccess, UPlotRoom*, Room, const FString&, Error);

/** Connection options for UPlotClient. */
USTRUCT(BlueprintType)
struct FPlotOptions
{
	GENERATED_BODY()

	/** Publishable app key, e.g. "pl_pub_live_xxx". */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
	FString AppKey;

	/** Stable per-player identifier. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
	FString PlayerId;

	/** REST API base URL. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
	FString ApiUrl = TEXT("https://api.plot.ws");

	/** Optional reconnect token from a previous session. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
	FString Token;
};

/**
 * Entry point for the Plot SDK. A UGameInstanceSubsystem, so one instance lives
 * for the lifetime of the GameInstance and is reachable from anywhere via
 * `GetGameInstance()->GetSubsystem<UPlotClient>()`.
 *
 * Usage: Configure() once, then Join() with a room code. The two-step
 * handshake (REST connect -> WebSocket open with the X-Plot-Protocol header) is
 * performed for you; the OnResult delegate fires with a UPlotRoom on success.
 */
UCLASS(BlueprintType)
class PLOTCLIENT_API UPlotClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Set the app key / player id / api url before joining. */
	UFUNCTION(BlueprintCallable, Category = "Plot")
	void Configure(const FPlotOptions& InOptions);

	/**
	 * Connect (REST /v1/connect) then open the room WebSocket using the
	 * `X-Plot-Protocol: v1b.0` handshake header. `OnResult` is invoked with the
	 * live UPlotRoom on success, or an error string on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Plot")
	void Join(const FString& RoomCode, const FPlotJoinResult& OnResult);

	/** The protocol version this SDK speaks. */
	UFUNCTION(BlueprintPure, Category = "Plot")
	static FString ProtocolVersion();

private:
	void OpenRoom(const FString& RoomCode, const FString& Token, const FString& WsUrl, FPlotJoinResult OnResult);

	FPlotOptions Options;
};
