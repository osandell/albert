// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "applicationbase.h"
#include <QString>
#include <QStringList>
#include <QUrl>
#include <vector>
#include <memory>
#include <albert/item.h>

class Application : public ApplicationBase
{
public:

    struct ParseOptions
    {
        bool ignore_show_in_keys;
        bool use_exec;
        bool use_generic_name;
        bool use_keywords;
        bool use_non_localized_name;
    };

    Application(const QString &id, const QString &path, ParseOptions po);
    Application(const Application &) = default;

    QString subtext() const override;
    std::unique_ptr<albert::Icon> icon() const override;
    void launch() const override;
    std::vector<albert::Action> actions() const override;

    const QStringList &exec() const;

    bool isTerminal() const;

protected:

    void launchExec(const QStringList &exec, QUrl url, const QString &working_dir) const;

private:

    QString description_;
    QString icon_;
    QStringList exec_;
    QString working_dir_;
    bool is_terminal_ = false;

};

