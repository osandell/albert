// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/item.h>
#include <memory>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::util::IndexQueryHandler
{
    ALBERT_PLUGIN
public:

    Plugin();
    ~Plugin();
    void updateIndexItems() override;

private:

    class Private;
    std::unique_ptr<Private> d;
    
};
