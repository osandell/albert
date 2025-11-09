// Copyright (c) 2023-2024 Manuel Schneider
// Windows implementation - no Unix signals needed

#include "signalhandler.h"

SignalHandler::SignalHandler()
{
    // Windows doesn't use Unix signals (SIGTERM, SIGINT, etc.)
    // Qt handles WM_CLOSE and other window messages automatically for GUI apps
    // Console control handlers (Ctrl+C) are only needed for console apps
}

SignalHandler::~SignalHandler()
{
    // Nothing to clean up on Windows
}

