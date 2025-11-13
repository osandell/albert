// Copyright (c) 2024-2025 Manuel Schneider

#include "theme.h"
#include <albert/logging.h>
#include <QApplication>
#include <QSettings>
#include <QStyle>
#include <map>
using namespace Qt::StringLiterals;
using namespace std;


Theme::Theme():
    Theme(QApplication::style()->standardPalette())
{}

Theme::Theme(const QPalette &p):
    palette(p),
    // Do sync with template.ini
    window_shadow_brush                     (QColor(0, 0, 0, 128)),
    window_background_brush                 (p.brush(QPalette::Window)),
    window_border_brush                     (p.brush(QPalette::Highlight)),
    input_background_brush                  (p.brush(QPalette::Base)),
    input_border_brush                      (p.brush(QPalette::Highlight)),
    input_trigger_color                     (p.color(QPalette::Highlight)),
    input_hint_color                        (p.color(QPalette::Button)),
    settings_button_color                   (p.color(QPalette::Button)), // Placeholder is transparent on some systems
    settings_button_highlight_color         (p.color(QPalette::Highlight)),
    result_item_selection_background_brush  (p.brush(QPalette::Highlight)),
    result_item_selection_border_brush      (p.brush(QPalette::Highlight)),
    result_item_selection_text_color        (p.color(QPalette::HighlightedText)),
    result_item_selection_subtext_color     (p.color(QPalette::HighlightedText)),
    result_item_text_color                  (p.color(QPalette::WindowText)),
    result_item_subtext_color               (p.color(QPalette::PlaceholderText)),
    action_item_selection_background_brush  (p.brush(QPalette::Highlight)),
    action_item_selection_border_brush      (p.brush(QPalette::Highlight)),
    action_item_selection_text_color        (p.color(QPalette::HighlightedText)),
    action_item_text_color                  (p.color(QPalette::WindowText))
{

}

//--------------------------------------------------------------------------------------------------

namespace
{

struct {
    const QString base                                   = u"palette/base"_s;
    const QString text                                   = u"palette/text"_s;
    const QString window                                 = u"palette/window"_s;
    const QString window_text                            = u"palette/window_text"_s;
    const QString button                                 = u"palette/button"_s;
    const QString button_text                            = u"palette/button_text"_s;
    const QString light                                  = u"palette/light"_s;
    const QString mid                                    = u"palette/mid"_s;
    const QString dark                                   = u"palette/dark"_s;
    const QString placeholder_text                       = u"palette/placeholder_text"_s;
    const QString highlight                              = u"palette/highlight"_s;
    const QString highlight_text                         = u"palette/highlight_text"_s;
    const QString link                                   = u"palette/link"_s;
    const QString link_visited                           = u"palette/link_visited"_s;

    const QString window_shadow_brush                    = u"window/window_shadow_brush"_s;
    const QString window_background_brush                = u"window/window_background_brush"_s;
    const QString window_border_brush                    = u"window/window_border_brush"_s;
    const QString input_background_brush                 = u"window/input_background_brush"_s;
    const QString input_border_brush                     = u"window/input_border_brush"_s;
    const QString input_trigger_color                    = u"window/input_trigger_color"_s;
    const QString input_hint_color                       = u"window/input_hint_color"_s;
    const QString settings_button_color                  = u"window/settings_button_color"_s;
    const QString settings_button_highlight_color        = u"window/settings_button_highlight_color"_s;
    const QString result_item_selection_background_brush = u"window/result_item_selection_background_brush"_s;
    const QString result_item_selection_border_brush     = u"window/result_item_selection_border_brush"_s;
    const QString result_item_selection_text_color       = u"window/result_item_selection_text_color"_s;
    const QString result_item_selection_subtext_color    = u"window/result_item_selection_subtext_color"_s;
    const QString result_item_text_color                 = u"window/result_item_text_color"_s;
    const QString result_item_subtext_color              = u"window/result_item_subtext_color"_s;
    const QString action_item_selection_background_brush = u"window/action_item_selection_background_brush"_s;
    const QString action_item_selection_border_brush     = u"window/action_item_selection_border_brush"_s;
    const QString action_item_selection_text_color       = u"window/action_item_selection_text_color"_s;
    const QString action_item_text_color                 = u"window/action_item_text_color"_s;
} key;

}

static QBrush parseBrush(const QString &s)
{
    if (s.isEmpty())
        return {};

    else if (s[0] == u'#')
    {
        QColor c(s);
        return c.isValid() ? QBrush(c) : QBrush{};
    }

    // for (int cr = 0; cr < (int)QPalette::NColorRoles; ++cr)
    //     if (s == QMetaEnum::fromType<QPalette::ColorRole>().key(cr))
    //         return palette.color(QPalette::Active, static_cast<QPalette::ColorRole>(cr));

    else if (QColor c(s); c.isValid())
        return c;

    else
        return {};
}

Theme Theme::read(const QString &path)
{
    QSettings ini(path, QSettings::IniFormat);


    // Get all values

    map<QString, QString> kv;
    for (const auto &k : ini.allKeys())
        if (auto v = ini.value(k).toString().trimmed(); v.isEmpty())
            WARN << "Ignoring empty entry" << k;
        else
            kv.emplace(k, v);


    // Resolve and read values

    std::map<QString, QBrush> brushes;

    while(kv.size() > 0)
    {
        auto c = kv.size();

        // Parse all non references
        for (auto it = kv.begin(); it != kv.end();)
        {
            auto &[k, v] = *it;

            if (v[0] == u'$')
                ++it;

            else if (auto b = parseBrush(v); b.style() == Qt::NoBrush)
                throw runtime_error(QStringLiteral("Invalid brush for %1: %2").arg(k, v).toStdString());

            else
            {
                brushes.emplace(k, b);
                it = kv.erase(it);
            }
        }

        if (c == kv.size())
            throw runtime_error(QStringLiteral("Cyclic reference: %1").arg(kv.begin()->first).toStdString());

        // Resolve references (only references in kv)
        for (auto it = kv.begin(); it != kv.end();)
        {
            auto &[k, v] = *it;

            // if ref is already parsed resolve directly
            if (auto b_it = brushes.find(v.mid(1)); b_it != brushes.end())
            {
                brushes.emplace(k, b_it->second);
                it = kv.erase(it);
            }

            // if not already parsed and the key exists update reference
            else if (auto ref_it = kv.find(v.mid(1)); ref_it != kv.end())
            {
                v = ref_it->second;
                ++it;
            }

            // If neither is the case we have a dangling refernce;
            else
                throw runtime_error(QStringLiteral("Dangling reference: %1").arg(v).toStdString());
        }
    }


    // Read palette

    auto getThrow = [&brushes](const QString &role) {
        try {
            return brushes.at(role);
        } catch (const out_of_range &e) {
            throw runtime_error(QStringLiteral("Mandatory key missing: %1").arg(role).toStdString());
        }
    };

    auto base             = getThrow(key.base);
    auto text             = getThrow(key.text);
    auto window           = getThrow(key.window);
    auto window_text      = getThrow(key.window_text);
    auto button           = getThrow(key.button);
    auto button_text      = getThrow(key.button_text);
    auto highlight        = getThrow(key.highlight);
    auto highlight_text   = getThrow(key.highlight_text);
    auto placeholder_text = getThrow(key.placeholder_text);
    auto link             = getThrow(key.link);
    auto link_visited     = getThrow(key.link_visited);

    QBrush light, mid, dark;
    try
    {
        light = brushes.at(key.light);
        mid   = brushes.at(key.mid);
        dark  = brushes.at(key.dark);
    }
    catch (const out_of_range&)
    {
        light = button.color().lighter();
        mid   = button.color().darker();
        dark  = mid.color().darker();
    }

    QPalette palette(window_text, button, light, mid, dark, text, button_text, base, window);
    palette.setColor(QPalette::All, QPalette::Highlight, highlight.color());
    palette.setColor(QPalette::All, QPalette::HighlightedText, highlight_text.color());
    palette.setColor(QPalette::All, QPalette::Link, link.color());
    palette.setColor(QPalette::All, QPalette::LinkVisited, link_visited.color());
    palette.setColor(QPalette::All, QPalette::PlaceholderText, placeholder_text.color());


    // Initialize theme with palette

    auto theme = Theme(palette);


    // Read window colors

    auto setb = [&brushes](const QString &k, QBrush *out) {
        if (auto it = brushes.find(k); it != brushes.end())
            *out = it->second;
    };
    auto setc = [&brushes](const QString &k, QColor *out) {
        if (auto it = brushes.find(k); it != brushes.end())
            *out = it->second.color();
    };

    setb(key.window_shadow_brush,                    &theme.window_shadow_brush);
    setb(key.window_background_brush,                &theme.window_background_brush);
    setb(key.window_border_brush,                    &theme.window_border_brush);
    setb(key.input_background_brush,                 &theme.input_background_brush);
    setb(key.input_border_brush,                     &theme.input_border_brush);
    setc(key.input_trigger_color,                    &theme.input_trigger_color);
    setc(key.input_hint_color,                       &theme.input_hint_color);
    setc(key.settings_button_color,                  &theme.settings_button_color);
    setc(key.settings_button_highlight_color,        &theme.settings_button_highlight_color);
    setb(key.result_item_selection_background_brush, &theme.result_item_selection_background_brush);
    setb(key.result_item_selection_border_brush,     &theme.result_item_selection_border_brush);
    setc(key.result_item_selection_text_color,       &theme.result_item_selection_text_color);
    setc(key.result_item_selection_subtext_color,    &theme.result_item_selection_subtext_color);
    setc(key.result_item_text_color,                 &theme.result_item_text_color);
    setc(key.result_item_subtext_color,              &theme.result_item_subtext_color);
    setb(key.action_item_selection_background_brush, &theme.action_item_selection_background_brush);
    setb(key.action_item_selection_border_brush,     &theme.action_item_selection_border_brush);
    setc(key.action_item_selection_text_color,       &theme.action_item_selection_text_color);
    setc(key.action_item_text_color,                 &theme.action_item_text_color);

    return theme;
}
