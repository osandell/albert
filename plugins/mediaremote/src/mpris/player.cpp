// Copyright (c) 2017-2025 Manuel Schneider
// https://www.freedesktop.org/wiki/Specifications/mpris-spec/metadata/

#include "mpris.h"
#include "player.h"
#include <albert/desktopentryparser.h>
#include <albert/iconutil.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

static const auto dbus_timeout = 100;
static const auto dbus_object_path = u"/org/mpris/MediaPlayer2"_s;


static QString getDesktopEntry(const QString desktop_entry_basename)
{
    const auto xdg_data_dirs(qEnvironmentVariable("XDG_DATA_DIRS").split(u':', Qt::SkipEmptyParts));
    for (const auto &xdg_data_dir : xdg_data_dirs)
        if (const auto file_path = QString(u"%1/applications/%2.desktop"_s)
                                       .arg(xdg_data_dir, desktop_entry_basename);
            QFile::exists(file_path))
            return file_path;
    return {};
}

Player::Player(const QString &service_name):
    dbus_service_name(service_name),
    player(dbus_service_name, dbus_object_path, QDBusConnection::sessionBus()),
    control(dbus_service_name, dbus_object_path, QDBusConnection::sessionBus()),
    name_(player.identity())
{
    if (const auto file_path = getDesktopEntry(player.desktopEntry());
        file_path.isEmpty())
    {
        name_ = player.identity();
        icon_factory_ = []{ return makeThemeIcon(u"multimedia-player"_s); };
    }
    else
    {
        detail::DesktopEntryParser desktop_entry(file_path);
        name_ = desktop_entry.getLocaleString(u"Desktop Entry"_s, u"Name"_s);
        if (const auto icon_string = desktop_entry.getIconString(u"Desktop Entry"_s, u"Icon"_s);
            QFile::exists(icon_string))
            icon_factory_ = [=]{ return makeImageIcon(icon_string); };
        else
            icon_factory_ = [=]{ return makeThemeIcon(icon_string); };
    }

    player.setTimeout(dbus_timeout);
    control.setTimeout(dbus_timeout);
}

QString Player::name() const { return name_; }

unique_ptr<Icon> Player::icon() { return icon_factory_(); }

bool Player::isPlaying() const { return control.playbackStatus() == u"Playing"_s; }

bool Player::canPlay() const { return control.canPlay(); }

bool Player::canPause() const { return control.canPause(); }

bool Player::canGoNext() const { return control.canGoNext(); }

bool Player::canGoPrevious() const { return control.canGoPrevious(); }

void Player::play() { control.Play(); }

void Player::pause() { control.Pause(); }

void Player::next() { control.Next(); }

void Player::previous() { control.Previous(); }

// auto md = control.metadata();
// CRIT << md;
// QStringList sl;
// sl << playback_status_strings[playback_status];
// if (auto it1 = md.find("xesam:title"), it2 = md.find("xesam:artist");
//     it1 != md.end() && it2 != md.end() && it1->canConvert<QString>() && it2->canConvert<QString>())
// {
//     sl << it1->toString() << it2->toString();
// }
// else if (it1 = md.find("xesam:url"); it1 != md.end() && it1->canConvert<QString>())
// {
//     QFileInfo fi(QUrl(it1->toString()).toLocalFile());
//     sl << fi.fileName() << fi.dir().dirName();
// }
