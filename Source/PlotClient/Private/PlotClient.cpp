// Copyright Plot. SDK code: MIT.
#include "PlotClient.h"

#include "PlotClientModule.h"
#include "Protocol/PlotProtocol.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformHttp.h"

void UPlotClient::Configure(const FPlotOptions& InOptions)
{
	Options = InOptions;
}

FString UPlotClient::ProtocolVersion()
{
	return PLOT_SCHEMA_VERSION;
}

void UPlotClient::Join(const FString& RoomCode, const FPlotJoinResult& OnResult)
{
	if (RoomCode.IsEmpty())
	{
		OnResult.ExecuteIfBound(false, nullptr, TEXT("RoomCode required; matchmake-by-mode is not wired in the v1e SDK"));
		return;
	}
	if (Options.AppKey.IsEmpty() || Options.PlayerId.IsEmpty())
	{
		OnResult.ExecuteIfBound(false, nullptr, TEXT("Configure() with AppKey and PlayerId before Join()"));
		return;
	}

	// Build the REST connect request body.
	const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("appKey"), Options.AppKey);
	Body->SetStringField(TEXT("playerId"), Options.PlayerId);
	if (!Options.Token.IsEmpty())
	{
		Body->SetStringField(TEXT("token"), Options.Token);
	}
	FString BodyStr;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
	FJsonSerializer::Serialize(Body, Writer);

	const FString Url = Options.ApiUrl / TEXT("v1/connect");

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(PLOT_HANDSHAKE_HEADER, PLOT_SCHEMA_VERSION);
	Request->SetContentAsString(BodyStr);

	const FString CapturedRoom = RoomCode;
	TWeakObjectPtr<UPlotClient> WeakThis(this);
	FPlotJoinResult Result = OnResult;
	Request->OnProcessRequestComplete().BindLambda(
		[WeakThis, CapturedRoom, Result](FHttpRequestPtr /*Req*/, FHttpResponsePtr Response, bool bConnectedOk)
		{
			UPlotClient* Self = WeakThis.Get();
			if (!Self)
			{
				return;
			}
			if (!bConnectedOk || !Response.IsValid())
			{
				Result.ExecuteIfBound(false, nullptr, TEXT("connect request failed"));
				return;
			}
			const int32 Code = Response->GetResponseCode();
			if (Code < 200 || Code >= 300)
			{
				Result.ExecuteIfBound(false, nullptr, FString::Printf(TEXT("connect returned HTTP %d"), Code));
				return;
			}

			TSharedPtr<FJsonObject> Json;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
			{
				Result.ExecuteIfBound(false, nullptr, TEXT("connect returned malformed JSON"));
				return;
			}
			FString Token;
			FString WsUrl;
			Json->TryGetStringField(TEXT("token"), Token);
			Json->TryGetStringField(TEXT("wsUrl"), WsUrl);
			if (WsUrl.IsEmpty())
			{
				Result.ExecuteIfBound(false, nullptr, TEXT("connect response missing wsUrl"));
				return;
			}
			Self->OpenRoom(CapturedRoom, Token, WsUrl, Result);
		});
	Request->ProcessRequest();
}

void UPlotClient::OpenRoom(const FString& RoomCode, const FString& Token, const FString& WsUrl, FPlotJoinResult OnResult)
{
	FWebSocketsModule& WsModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");

	// Carry roomCode + token as query params, mirroring the other SDKs.
	const FString FullUrl = FString::Printf(
		TEXT("%s?roomCode=%s&token=%s"),
		*WsUrl,
		*FGenericPlatformHttp::UrlEncode(RoomCode),
		*FGenericPlatformHttp::UrlEncode(Token));

	// Send the protocol handshake as an upgrade-request header.
	const TMap<FString, FString> Headers = {
		{ FString(PLOT_HANDSHAKE_HEADER), FString(PLOT_SCHEMA_VERSION) },
	};

	TSharedRef<IWebSocket> Socket = WsModule.CreateWebSocket(FullUrl, FString(), Headers);

	UPlotRoom* Room = NewObject<UPlotRoom>(this);
	Room->Initialize(Options.PlayerId, Socket);

	FPlotJoinResult Result = OnResult;
	TWeakObjectPtr<UPlotRoom> WeakRoom(Room);

	Socket->OnConnected().AddLambda([WeakRoom, Result]()
	{
		Result.ExecuteIfBound(true, WeakRoom.Get(), FString());
	});
	Socket->OnConnectionError().AddLambda([Result](const FString& Error)
	{
		Result.ExecuteIfBound(false, nullptr, Error);
	});

	Socket->Connect();
}
