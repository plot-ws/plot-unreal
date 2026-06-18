// Copyright Plot. SDK code: MIT.
#include "PlotClientModule.h"

#include "Modules/ModuleManager.h"
#include "WebSocketsModule.h"

DEFINE_LOG_CATEGORY(LogPlot);

void FPlotClientModule::StartupModule()
{
	// Ensure the WebSockets module is loaded before any IWebSocket is created.
	FModuleManager::Get().LoadModuleChecked("WebSockets");
}

void FPlotClientModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FPlotClientModule, PlotClient)
