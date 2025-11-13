// Copyright (c) 2022-2025 Manuel Schneider

#pragma once

#include "themesqueryhandler.h"
#include "window.h"
#include <QString>
#include <albert/frontend.h>

class Plugin : public albert::detail::Frontend
{
    ALBERT_PLUGIN

public:

    Plugin();

    std::vector<albert::Extension*> extensions() override;

    bool isVisible() const override;
    void setVisible(bool visible) override;
    QString input() const override;
    void setInput(const QString&) override;
    QWidget* createFrontendConfigWidget() override;
    unsigned long long winId() const override;
    void setQuery(albert::Query *query) override;

private:

    Window window;
    ThemesQueryHandler themes_query_handler;

};
