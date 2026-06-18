// Copyright Plot. SDK code: MIT.
#include "PlotRoom.h"

#include "PlotClientModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UPlotRoom::Initialize(const FString& InPlayerId, const TSharedRef<IWebSocket>& InSocket)
{
	PlayerId = InPlayerId;
	Socket = InSocket;

	Socket->OnMessage().AddUObject(this, &UPlotRoom::HandleIncoming);
	Socket->OnClosed().AddLambda([WeakThis = TWeakObjectPtr<UPlotRoom>(this)](int32 StatusCode, const FString& Reason, bool /*bWasClean*/)
	{
		if (UPlotRoom* Self = WeakThis.Get())
		{
			Self->OnClosed.Broadcast(Reason);
		}
	});
}

void UPlotRoom::Send(const FString& DataJson, const FString& Channel)
{
	if (!Socket.IsValid() || !Socket->IsConnected())
	{
		UE_LOG(LogPlot, Warning, TEXT("UPlotRoom::Send called while disconnected"));
		return;
	}

	// Parse the caller's data payload so it is embedded as a real JSON value
	// (object/array/scalar) rather than a double-encoded string. Fall back to a
	// JSON string if it is not valid JSON on its own.
	TSharedPtr<FJsonValue> DataValue;
	{
		const FString Wrapped = FString::Printf(TEXT("{\"v\":%s}"), *DataJson);
		TSharedPtr<FJsonObject> Probe;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Wrapped);
		if (FJsonSerializer::Deserialize(Reader, Probe) && Probe.IsValid())
		{
			DataValue = Probe->TryGetField(TEXT("v"));
		}
	}
	if (!DataValue.IsValid())
	{
		DataValue = MakeShared<FJsonValueString>(DataJson);
	}

	const TSharedRef<FJsonObject> Envelope = MakeShared<FJsonObject>();
	Envelope->SetStringField(TEXT("type"), TEXT("message"));
	Envelope->SetStringField(TEXT("channel"), Channel.IsEmpty() ? TEXT("event") : Channel);
	Envelope->SetField(TEXT("data"), DataValue.ToSharedRef());
	Envelope->SetNumberField(TEXT("clientTs"), static_cast<double>(NowMs()));

	FString Out;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Envelope, Writer);
	Socket->Send(Out);
}

void UPlotRoom::Leave()
{
	if (Socket.IsValid())
	{
		if (Socket->IsConnected())
		{
			Socket->Close(1000, TEXT("client closed"));
		}
		Socket.Reset();
	}
	OnMessage.Clear();
	OnPlayerJoined.Clear();
	OnPlayerLeft.Clear();
	OnClosed.Clear();
}

bool UPlotRoom::IsConnected() const
{
	return Socket.IsValid() && Socket->IsConnected();
}

void UPlotRoom::HandleIncoming(const FString& Json)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogPlot, Warning, TEXT("UPlotRoom: dropped malformed frame"));
		return;
	}

	FString Type;
	if (!Root->TryGetStringField(TEXT("type"), Type))
	{
		return;
	}

	if (Type == TEXT("message"))
	{
		const FString From = Root->HasField(TEXT("from")) ? Root->GetStringField(TEXT("from")) : FString();
		const FString Channel = Root->HasField(TEXT("channel")) ? Root->GetStringField(TEXT("channel")) : TEXT("event");

		FString DataJson;
		if (const TSharedPtr<FJsonValue> DataField = Root->TryGetField(TEXT("data")))
		{
			const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DataJson);
			FJsonSerializer::Serialize(DataField.ToSharedRef(), TEXT(""), Writer);
		}
		OnMessage.Broadcast(From, Channel, DataJson);
	}
	else if (Type == TEXT("join"))
	{
		FString JoinedId;
		Root->TryGetStringField(TEXT("playerId"), JoinedId);
		TArray<FString> Players;
		const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
		if (Root->TryGetArrayField(TEXT("players"), Arr) && Arr)
		{
			for (const TSharedPtr<FJsonValue>& V : *Arr)
			{
				Players.Add(V->AsString());
			}
		}
		OnPlayerJoined.Broadcast(JoinedId, Players);
	}
	else if (Type == TEXT("leave"))
	{
		FString LeftId;
		Root->TryGetStringField(TEXT("playerId"), LeftId);
		TArray<FString> Players;
		const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
		if (Root->TryGetArrayField(TEXT("players"), Arr) && Arr)
		{
			for (const TSharedPtr<FJsonValue>& V : *Arr)
			{
				Players.Add(V->AsString());
			}
		}
		OnPlayerLeft.Broadcast(LeftId, Players);
	}
	// state-snapshot / state-patch / reconnect-token / error are part of the
	// wire protocol but the BASELINE client surface (parity with the other
	// SDKs' connect/join/send/receive) does not expose them. Interpolation and
	// prediction are explicitly out of scope for the Unreal phase-2 item.
}

int64 UPlotRoom::NowMs()
{
	return static_cast<int64>(FDateTime::UtcNow().ToUnixTimestamp()) * 1000
		+ FDateTime::UtcNow().GetMillisecond();
}
