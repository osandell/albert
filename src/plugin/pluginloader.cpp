// Copyright (c) 2024-2025 Manuel Schneider

#include "pluginloader.h"
using namespace albert;

namespace {
    thread_local PluginLoader *g_current_loader = nullptr;
}

PluginLoader *PluginLoader::current_loader()
{
    return g_current_loader;
}

void PluginLoader::set_current_loader(PluginLoader *loader)
{
    g_current_loader = loader;
}

PluginLoader::~PluginLoader() = default;
