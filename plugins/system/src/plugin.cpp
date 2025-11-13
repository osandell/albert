// Copyright (c) 2017-2025 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <albert/extensionregistry.h>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
ALBERT_LOGGING_CATEGORY("system")
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;
using enum SupportedCommands;

static QString defaultCommand(SupportedCommands command)
{
#if defined(Q_OS_MAC)
    switch (command) {
        case LOCK:      return uR"(pmset displaysleepnow)"_s;
        case LOGOUT:    return uR"(osascript -e 'tell app "System Events" to log out')"_s;
        case SUSPEND:   return uR"(osascript -e 'tell app "System Events" to sleep')"_s;
        case REBOOT:    return uR"(osascript -e 'tell app "System Events" to restart')"_s;
        case POWEROFF:  return uR"(osascript -e 'tell app "System Events" to shut down')"_s;
        }
#elif defined(Q_OS_UNIX)
    for (const QString &de : qEnvironmentVariable("XDG_CURRENT_DESKTOP").split(u':')) {

        if (de == u"Unity"_s || de == u"Pantheon"_s || de == u"GNOME"_s)
            switch (command) {
            case LOCK:      return u"dbus-send --type=method_call --dest=org.gnome.ScreenSaver /org/gnome/ScreenSaver org.gnome.ScreenSaver.Lock"_s;
            case LOGOUT:    return u"gnome-session-quit --logout --no-prompt"_s;
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return u"gnome-session-quit --reboot --no-prompt"_s;
            case POWEROFF:  return u"gnome-session-quit --power-off --no-prompt"_s;
            }

        else if (de == u"kde-plasma"_s || de == u"KDE"_s)
            switch (command) {
            case LOCK:      return u"dbus-send --type=method_call --dest=org.freedesktop.ScreenSaver /ScreenSaver org.freedesktop.ScreenSaver.Lock"_s;
            case LOGOUT:    return u"dbus-send --session --type=method_call --dest=org.kde.Shutdown /Shutdown org.kde.Shutdown.logout"_s;
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return u"dbus-send --session --type=method_call --dest=org.kde.Shutdown /Shutdown org.kde.Shutdown.logoutAndReboot"_s;
            case POWEROFF:  return u"dbus-send --session --type=method_call --dest=org.kde.Shutdown /Shutdown org.kde.Shutdown.logoutAndShutdown"_s;
            }

        else if (de == u"X-Cinnamon"_s || de == u"Cinnamon"_s)
            switch (command) {
            case LOCK:      return u"cinnamon-screensaver-command --lock"_s;
            case LOGOUT:    return u"cinnamon-session-quit --logout"_s;
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return u"cinnamon-session-quit --reboot"_s;
            case POWEROFF:  return u"cinnamon-session-quit --power-off"_s;
            }

        else if (de == u"MATE"_s)
            switch (command) {
            case LOCK:      return u"mate-screensaver-command --lock"_s;
            case LOGOUT:    return u"mate-session-save --logout-dialog"_s;
            case SUSPEND:   return u"sh -c \"mate-screensaver-command --lock && systemctl suspend -i\""_s;
            case HIBERNATE: return u"sh -c \"mate-screensaver-command --lock && systemctl hibernate -i\""_s;
            case REBOOT:    return u"mate-session-save --shutdown-dialog"_s;
            case POWEROFF:  return u"mate-session-save --shutdown-dialog"_s;
            }

        else if (de == u"XFCE"_s)
            switch (command) {
            case LOCK:      return u"xflock4"_s;
            case LOGOUT:    return u"xfce4-session-logout --logout"_s;
            case SUSPEND:   return u"xfce4-session-logout --suspend"_s;
            case HIBERNATE: return u"xfce4-session-logout --hibernate"_s;
            case REBOOT:    return u"xfce4-session-logout --reboot"_s;
            case POWEROFF:  return u"xfce4-session-logout --halt"_s;
            }

        else if (de == u"LXQt"_s)
            switch (command) {
            case LOCK:      return u"lxqt-leave --lockscreen"_s;
            case LOGOUT:    return u"lxqt-leave --logout"_s;
            case SUSPEND:   return u"lxqt-leave --suspend"_s;
            case HIBERNATE: return u"lxqt-leave --hibernate"_s;
            case REBOOT:    return u"lxqt-leave --reboot"_s;
            case POWEROFF:  return u"lxqt-leave --shutdown"_s;
            }
    }
    switch (command) {
    case LOCK:      return u"xdg-screensaver lock"_s;
    case LOGOUT:    return u"notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out"_s;
    case SUSPEND:   return u"systemctl suspend -i"_s;
    case HIBERNATE: return u"systemctl hibernate -i"_s;
    case REBOOT:    return u"notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot"_s;
    case POWEROFF:  return u"notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown"_s;
    } 
#endif
    return {};
}

Plugin::Plugin():
    commands({
        Command{
         .id = LOCK,
         .config_key_enabled = u"lock_enabled"_s,
         .config_key_title = u"title_lock"_s,
         .config_key_command = u"command_lock"_s,
         .icon_name = u"system-lock-screen"_s,
         .default_title = tr("Lock"),
         .description = tr("Lock the session"),
         .command = defaultCommand(LOCK),
        },
        Command{
         .id = LOGOUT,
         .config_key_enabled = u"logout_enabled"_s,
         .config_key_title = u"title_logout"_s,
         .config_key_command = u"command_logout"_s,
         .icon_name = u"system-log-out"_s,
         .default_title = tr("Logout"),
         .description = tr("Quit the session"),
         .command = defaultCommand(LOGOUT),
        },
        Command{
         .id = SUSPEND,
         .config_key_enabled = u"suspend_enabled"_s,
         .config_key_title = u"title_suspend"_s,
         .config_key_command = u"command_suspend"_s,
         .icon_name = u"system-suspend"_s,
         .default_title = tr("Suspend"),
         .description = tr("Suspend to memory"),
         .command = defaultCommand(SUSPEND),
        },
#if not defined(Q_OS_MAC)
        Command{
         .id = HIBERNATE,
         .config_key_enabled = u"hibernate_enabled"_s,
         .config_key_title = u"title_hibernate"_s,
         .config_key_command = u"command_hibernate"_s,
         .icon_name = u"system-suspend-hibernate"_s,
         .default_title = tr("Hibernate"),
         .description = tr("Suspend to disk"),
         .command = defaultCommand(HIBERNATE),
        },
#endif
        Command{
         .id = REBOOT,
         .config_key_enabled = u"reboot_enabled"_s,
         .config_key_title = u"title_reboot"_s,
         .config_key_command = u"command_reboot"_s,
         .icon_name = u"system-reboot"_s,
         .default_title = tr("Reboot"),
         .description = tr("Restart the machine"),
         .command = defaultCommand(REBOOT),
        },
        Command{
         .id = POWEROFF,
         .config_key_enabled = u"poweroff_enabled"_s,
         .config_key_title = u"title_poweroff"_s,
         .config_key_command = u"command_poweroff"_s,
         .icon_name = u"system-shutdown"_s,
         .default_title = tr("Poweroff"),
         .description = tr("Shut down the machine"),
         .command = defaultCommand(POWEROFF),
        }
    })
{}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    auto s = settings();
    for (uint i = 0; i < commands.size(); ++i)
    {
        const auto &c = commands[i];

        auto *checkbox = new QCheckBox(w);
        auto *label = new QLabel(c.description, w);
        auto *line_edit_title = new QLineEdit(w);
        auto *line_edit_command = new QLineEdit(w);

        bool enabled = s->value(c.config_key_enabled, true).toBool();

        checkbox->setCheckState(enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        connect(checkbox, &QCheckBox::clicked, this, [=, this, c=&commands[i]](bool checked)
        {
            settings()->setValue(c->config_key_enabled, checked);

            // Restore defaults if unchecked
            if (!checked){
                settings()->remove(c->config_key_title);
                settings()->remove(c->config_key_command);
                line_edit_title->clear();
                line_edit_command->clear();
            }

            label->setEnabled(checked);
            line_edit_title->setEnabled(checked);
            line_edit_command->setEnabled(checked);

            updateIndexItems();
        });

        label->setEnabled(enabled);

        line_edit_title->setEnabled(enabled);
        // line_edit_title->setClearButtonEnabled(true);
        line_edit_title->setFixedWidth(100);
        line_edit_title->setPlaceholderText(c.default_title);
        line_edit_title->setText(s->value(c.config_key_title).toString());
        connect(line_edit_title, &QLineEdit::editingFinished,
                this, [this, line_edit_title, ck=c.config_key_title]
        {
            if (line_edit_title->text().isEmpty())
                settings()->remove(ck);
            else
                settings()->setValue(ck, line_edit_title->text());
            updateIndexItems();
        });

        line_edit_command->setEnabled(enabled);
        // line_edit_command->setClearButtonEnabled(true);
        line_edit_command->setPlaceholderText(defaultCommand(c.id));
        line_edit_command->setText(s->value(c.config_key_command).toString());
        connect(line_edit_command, &QLineEdit::editingFinished,
                this, [this, line_edit_command, ck=c.config_key_command]
        {
            if (line_edit_command->text().isEmpty())
                settings()->remove(ck);
            else
                settings()->setValue(ck, line_edit_command->text());
            updateIndexItems();
        });

        ui.gridLayout_commands->addWidget(checkbox, i * 2, 0);
        ui.gridLayout_commands->addWidget(label, i * 2, 1, 1, 2);
        ui.gridLayout_commands->addWidget(line_edit_title, i * 2 + 1, 1);
        ui.gridLayout_commands->addWidget(line_edit_command, i * 2 + 1, 2);
    }

    ui.verticalLayout->addStretch();

    return w;
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> index_items;
    auto s = settings();

    for (const auto &c : commands)
    {
        // skip if disabled
        if (!s->value(c.config_key_enabled, true).toBool())
            continue;

        auto item = StandardItem::make(
            c.default_title,
            settings()->value(c.config_key_title, c.default_title).toString(),
            c.description,
            [icon_string=c.icon_name]{ return makeThemeIcon(icon_string); },
            {
                {
                    c.default_title, c.description,
                    [this, &c](){ runDetachedProcess({
                        u"/bin/sh"_s, u"-c"_s,
                        settings()->value(c.config_key_command, defaultCommand(c.id)).toString()});
                    }
                }
            }
        );

        index_items.emplace_back(::move(item), item->text());
    }

    setIndexItems(::move(index_items));
}
