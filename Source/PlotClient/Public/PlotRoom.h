// Copyright Plot. SDK code: MIT.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "PlotRoom.generated.h"

/**
 * Fired when a `message` envelope arrives. `From` is the sender's player id,
 * `Channel` the message channel ("event"/"chat"/"state"/"unreliable"), and
 * `DataJson` the raw JSON of the `data` payload (parse it with your own struct
 * via FJsonObjectConverter, or treat it as a string).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FPlotOnMessage, const FString&, From, const FString&, Channel, const FString&, DataJson);

/** Fired on a `join` envelope. `PlayerId` joined; `Players` is the new roster. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPlotOnPlayerJoined, const FString&, PlayerId, const TArray<FString>&, Players);

/** Fired on a `leave` envelope. `PlayerId` left; `Players` is the new roster. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPlotOnPlayerLeft, const FString&, PlayerId, const TArray<FString>&, Players);

/** Fired when the underlying WebSocket closes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlotOnClosed, const FString&, Reason);

/**
 * A live room connection. Created by UPlotClient::Join; do not construct
 * directly. Wraps an IWebSocket and dispatches inbound envelopes to the
 * Blueprint-assignable events below.
 */
UCLASS(BlueprintType)
class PLOTCLIENT_API UPlotRoom : public UObject
{
	GENERATED_BODY()

public:
	/** Broadcast when a chat/event message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Plot")
	FPlotOnMessage OnMessage;

	/** Broadcast when a player joins the room. */
	UPROPERTY(BlueprintAssignable, Category = "Plot")
	FPlotOnPlayerJoined OnPlayerJoined;

	/** Broadcast when a player leaves the room. */
	UPROPERTY(BlueprintAssignable, Category = "Plot")
	FPlotOnPlayerLeft OnPlayerLeft;

	/** Broadcast when the connection closes. */
	UPROPERTY(BlueprintAssignable, Category = "Plot")
	FPlotOnClosed OnClosed;

	/**
	 * Send a message to the room. `DataJson` is a JSON-encoded payload (object,
	 * array, or scalar); it is embedded verbatim as the envelope `data` field.
	 * `Channel` defaults to "event". `clientTs` is stamped automatically.
	 */
	UFUNCTION(BlueprintCallable, Category = "Plot")
	void Send(const FString& DataJson, const FString& Channel = TEXT("event"));

	/** Close the connection and clear all event bindings. */
	UFUNCTION(BlueprintCallable, Category = "Plot")
	void Leave();

	/** True while the WebSocket is connected. */
	UFUNCTION(BlueprintCallable, Category = "Plot")
	bool IsConnected() const;

	/** The local player's id, as supplied to UPlotClient::Join. */
	UPROPERTY(BlueprintReadOnly, Category = "Plot")
	FString PlayerId;

	/** Wire the room to an already-opened socket. Called by UPlotClient. */
	void Initialize(const FString& InPlayerId, const TSharedRef<IWebSocket>& InSocket);

private:
	void HandleIncoming(const FString& Json);
	static int64 NowMs();

	TSharedPtr<IWebSocket> Socket;
};
