// AUTO-GENERATED — do not edit. Run `pnpm --filter @plot/protocol codegen`.
// SCHEMA_VERSION = v1b.0
#pragma once

#include "CoreMinimal.h"
#include "PlotProtocol.generated.h"

#define PLOT_SCHEMA_VERSION TEXT("v1b.0")
#define PLOT_HANDSHAKE_HEADER TEXT("X-Plot-Protocol")

UENUM(BlueprintType)
enum class EPlotChannel : uint8
{
    State UMETA(DisplayName = "state"),
    Event UMETA(DisplayName = "event"),
    Chat UMETA(DisplayName = "chat"),
    Unreliable UMETA(DisplayName = "unreliable")
};

USTRUCT(BlueprintType)
struct FPlotConnectRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString AppKey;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Token;
};

USTRUCT(BlueprintType)
struct FPlotConnectResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Token;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 ExpiresAt = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString WsUrl;
};

USTRUCT(BlueprintType)
struct FPlotJoinEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("join");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    TArray<FString> Players;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Ts = 0;
};

USTRUCT(BlueprintType)
struct FPlotLeaveEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("leave");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    TArray<FString> Players;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Ts = 0;
};

USTRUCT(BlueprintType)
struct FPlotMessageEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("message");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString From;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Channel = TEXT("event");

    // `data` is an opaque JSON payload; carried as a serialized JSON string.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Data;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Ts = 0;
};

USTRUCT(BlueprintType)
struct FPlotStateSnapshotEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("state-snapshot");

    // `state` is an opaque JSON payload; carried as a serialized JSON string.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString State;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Ts = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 LastAckedSeq = 0;
};

USTRUCT(BlueprintType)
struct FPlotStatePatchEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("state-patch");

    // `patch` is an opaque JSON payload; carried as a serialized JSON string.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Patch;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Ts = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 LastAckedSeq = 0;
};

USTRUCT(BlueprintType)
struct FPlotReconnectTokenEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("reconnect-token");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Token;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 ExpiresAt = 0;
};

USTRUCT(BlueprintType)
struct FPlotErrorEnvelope
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("error");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Code;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Message;
};

USTRUCT(BlueprintType)
struct FPlotClientMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Type = TEXT("message");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Channel = TEXT("event");

    // `data` is an opaque JSON payload; carried as a serialized JSON string.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    FString Data;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 Seq = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plot")
    int64 ClientTs = 0;
};
