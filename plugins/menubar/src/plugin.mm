// Copyright (c) 2024-2025 Manuel Schneider

#include "plugin.h"
#include <Cocoa/Cocoa.h>
#include <QKeySequence>
#include <QMessageBox>
#include <QUrl>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <mutex>
#include <shared_mutex>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;
ALBERT_LOGGING_CATEGORY("menu")
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

static Qt::KeyboardModifiers toQt(AXMenuItemModifiers command_modifiers)
{
    Qt::KeyboardModifiers qt_modifiers;

    if (command_modifiers & kAXMenuItemModifierShift)
        qt_modifiers.setFlag(Qt::ShiftModifier);

    if (command_modifiers & kAXMenuItemModifierOption)
        qt_modifiers.setFlag(Qt::AltModifier);

    if (command_modifiers & kAXMenuItemModifierControl)
        qt_modifiers.setFlag(Qt::MetaModifier);

    if (!(command_modifiers & kAXMenuItemModifierNoCommand))
        qt_modifiers.setFlag(Qt::ControlModifier); // see AXMenuItemModifiers

    return qt_modifiers;
}

// See
// https://github.com/216k155/MacOSX-SDKs/blob/master/MacOSX10.11.sdk/System/Library/Frameworks/
//   Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Menus.h
// https://github.com/Hammerspoon/hammerspoon/blob/master/extensions/application/application.lua
static const map<char, const QString> glyph_map
{
    // {0x00, u""_s},  // Null (always glyph 1)
    {0x02, u"⇥"_s},  // Tab to the right key (for left-to-right script systems)
    {0x03, u"⇤"_s},  // Tab to the left key (for right-to-left script systems)
    {0x04, u"⌤"_s},  // Enter key
    {0x05, u"⇧"_s},  // Shift key
    {0x06, u"⌃"_s},  // Control key
    {0x07, u"⌥"_s},  // Option key
    {0x09, u"␣"_s},  // Space (always glyph 3) key
    {0x0A, u"⌦"_s},  // Delete to the right key (for right-to-left script systems)
    {0x0B, u"↩"_s},  // Return key (for left-to-right script systems)
    {0x0C, u"↪"_s},  // Return key (for right-to-left script systems)
    {0x0D, u"↩"_s},  // Nonmarking return key
    {0x0F, u""_s},  // Pencil key
    {0x10, u"⇣"_s},  // Downward dashed arrow key
    {0x11, u"⌘"_s},  // Command key
    {0x12, u"✓"_s},  // Checkmark key
    {0x13, u"◇"_s},  // Diamond key
    {0x14, u""_s},  // Apple logo key (filled)
    // {0x15, u""_s},  // Unassigned (paragraph in Korean)
    {0x17, u"⌫"_s},  // Delete to the left key (for left-to-right script systems)
    {0x18, u"⇠"_s},  // Leftward dashed arrow key
    {0x19, u"⇡"_s},  // Upward dashed arrow key
    {0x1A, u"⇢"_s},  // Rightward dashed arrow key
    {0x1B, u"⎋"_s},  // Escape key
    {0x1C, u"⌧"_s},  // Clear key
    {0x1D, u"『"_s},  // Unassigned (left double quotes in Japanese)
    {0x1E, u"』"_s},  // Unassigned (right double quotes in Japanese)
    // {0x1F, u""_s},  // Unassigned (trademark in Japanese)
    {0x61, u"␢"_s},  // Blank key
    {0x62, u"⇞"_s},  // Page up key
    {0x63, u"⇪"_s},  // Caps lock key
    {0x64, u"←"_s},  // Left arrow key
    {0x65, u"→"_s},  // Right arrow key
    {0x66, u"↖"_s},  // Northwest arrow key
    {0x67, u"﹖"_s},  // Help key
    {0x68, u"↑"_s},  // Up arrow key
    {0x69, u"↘"_s},  // Southeast arrow key
    {0x6A, u"↓"_s},  // Down arrow key
    {0x6B, u"⇟"_s},  // Page down key
    {0x6C, u""_s},  // Apple logo key (outline)
    {0x6D, u""_s},  // Contextual menu key
    {0x6E, u"⌽"_s},  // Power key
    {0x6F, u"F1"_s},  // F1 key
    {0x70, u"F2"_s},  // F2 key
    {0x71, u"F3"_s},  // F3 key
    {0x72, u"F4"_s},  // F4 key
    {0x73, u"F5"_s},  // F5 key
    {0x74, u"F6"_s},  // F6 key
    {0x75, u"F7"_s},  // F7 key
    {0x76, u"F8"_s},  // F8 key
    {0x77, u"F9"_s},  // F9 key
    {0x78, u"F10"_s},  // F10 key
    {0x79, u"F11"_s},  // F11 key
    {0x7A, u"F12"_s},  // F12 key
    {0x87, u"F13"_s},  // F13 key
    {0x88, u"F14"_s},  // F14 key
    {0x89, u"F15"_s},  // F15 key
    {0x8A, u"⎈"_s},  // Control key (ISO standard)
    {0x8C, u"⏏"_s},  // Eject key (available on Mac OS X 10.2 and later)
    {0x8D, u"英数"_s},  // Japanese eisu key (available in Mac OS X 10.4 and later)
    {0x8E, u"かな"_s},  // Japanese kana key (available in Mac OS X 10.4 and later)
    {0x8F, u"F16"_s},  // F16 key (available in SnowLeopard and later)
    {0x90, u"F17"_s},  // F17 key (available in SnowLeopard and later)
    {0x91, u"F18"_s},  // F18 key (available in SnowLeopard and later)
    {0x92, u"F19"_s}   // F19 key (available in SnowLeopard and later)
};

struct MenuItem : public albert::Item
{
    MenuItem(AXUIElementRef element, QStringList path, const QString &shortcut, unique_ptr<Icon> icon)
        : element_(element)
        , path_(path)
        , shortcut_(shortcut)
        , icon_(::move(icon))
    {
        CFRetain(element_);
    }

    ~MenuItem() { CFRelease(element_); };

    QString id() const override { return path_.join(QString()); }

    QString text() const override { return path_.last(); }

    QString subtext() const override {
        return shortcut_.isEmpty() ?
                   pathString() : u"%1 (%2)"_s.arg(pathString(), shortcut_);
    }

    unique_ptr<Icon> icon() const override { return icon_->clone(); }

    QString inputActionText() const override { return text(); }

    vector<albert::Action> actions() const override
    {
        return {{
            u"activate"_s, Plugin::tr("Activate"),
            [this] {
                if (auto err = AXUIElementPerformAction(element_, kAXPressAction);
                    err != kAXErrorSuccess)
                    WARN << "Failed to activate menu item";
            }
        }};
    }

    QString pathString() const { return path_.join(u" → "_s); }

    AXUIElementRef element_;
    const QStringList path_;
    const QString shortcut_;
    const unique_ptr<Icon> icon_;
};

Qt::KeyboardModifiers convertCFModifiersToQtModifiers(int cfModifiers)
{
    Qt::KeyboardModifiers qtModifiers = Qt::NoModifier;
    if (cfModifiers & kCGEventFlagMaskShift)       qtModifiers |= Qt::ShiftModifier;
    if (cfModifiers & kCGEventFlagMaskControl)     qtModifiers |= Qt::ControlModifier;
    if (cfModifiers & kCGEventFlagMaskAlternate)   qtModifiers |= Qt::AltModifier;
    if (cfModifiers & kCGEventFlagMaskCommand)     qtModifiers |= Qt::MetaModifier;
    if (cfModifiers & kCGEventFlagMaskSecondaryFn) qtModifiers |= Qt::GroupSwitchModifier;
    return qtModifiers;
}

static void retrieveMenuItemsRecurse(const bool & valid,
                                     vector<shared_ptr<MenuItem>>& items,
                                     const Icon &icon,
                                     QStringList path,
                                     AXUIElementRef element)
{
    if (!valid)
        return;

    // Define attribute names to fetch at once
    enum AXKeys {
        Enabled,
        Title,
        Children,
        MenuItemCmdChar,
        MenuItemCmdGlyph,
        MenuItemCmdModifiers,
    };
    CFStringRef ax_attributes[] = {
        kAXEnabledAttribute,
        kAXTitleAttribute,
        kAXChildrenAttribute,
        kAXMenuItemCmdCharAttribute,
        kAXMenuItemCmdGlyphAttribute,
        kAXMenuItemCmdModifiersAttribute,
    };
    CFArrayRef attributes_array = CFArrayCreate(nullptr,
                                               (const void **) ax_attributes,
                                               sizeof(ax_attributes) / sizeof(ax_attributes[0]),
                                               &kCFTypeArrayCallBacks);
    CFArrayRef attribute_values = nullptr;
    auto error = AXUIElementCopyMultipleAttributeValues(element,
                                                        attributes_array,
                                                        0,
                                                        &attribute_values);

    if (error != kAXErrorSuccess)
        WARN << u"Failed to retrieve multiple attributes: %1 (See AXError.h)"_s.arg(error);
    else if(!attribute_values)
        WARN << "Failed to retrieve multiple attributes: Returned null.";
    else if(CFGetTypeID(attribute_values) != CFArrayGetTypeID())
        WARN << "Failed to retrieve multiple attributes: Returned type is not array.";
    else{

        class skip : exception {};

        try {

            // Get enabled state (kAXEnabledAttribute)

            auto value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Enabled);
            if (!value)
                throw runtime_error("Fetched kAXEnabledAttribute is null");
            else if (CFGetTypeID(value) == kAXValueAXErrorType)
                throw runtime_error("Fetched kAXEnabledAttribute is kAXValueAXErrorType");
            else if (CFGetTypeID(value) != CFBooleanGetTypeID())
                throw runtime_error("Fetched kAXEnabledAttribute is not of type CFBooleanRef");
            else if (!CFBooleanGetValue((CFBooleanRef)value))  // Skip disabled ones
                // throw runtime_error("AXUIElement is disabled (kAXEnabledAttribute)");
                throw skip();


            // Get title (kAXTitleAttribute), skip empty titles
            // Title is optional for recursion but mandatory for items

            QString title;

            try {
                value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Title);
                if (!value)
                    throw runtime_error("Fetched kAXTitleAttribute is null");

                else if (CFGetTypeID(value) == kAXValueAXErrorType)
                    throw runtime_error("Fetched kAXTitleAttribute is kAXValueAXErrorType");

                // expected, menus
                else if (CFGetTypeID(value) == CFStringGetTypeID()
                         && CFStringGetLength((CFStringRef)value) == 0)
                    throw runtime_error("AXUIElement title is empty");

                // expected, menus, maybe coordinates or sth
                // else if (CFGetTypeID(value) == AXValueGetTypeID());

                else if (CFGetTypeID(value) == CFStringGetTypeID())
                {
                    title = QString::fromCFString((CFStringRef)value).trimmed();
                    path << title;
                }

                // expected, menus have no title
                // else if (CFGetTypeID(value) != CFStringGetTypeID())


            } catch (const exception &e) {
                // path << "N/A";
                WARN << e.what();
            }


            // Get children (kAXChildrenAttribute)

            value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Children);
            if (!value)
                throw runtime_error("Fetched kAXChildrenAttribute is null");
            else if (CFGetTypeID(value) == kAXValueAXErrorType)
                throw runtime_error("Fetched kAXChildrenAttribute is kAXValueAXErrorType");
            else if (CFGetTypeID(value) != CFArrayGetTypeID())
                throw runtime_error("Fetched kAXChildrenAttribute is not of type CFArrayRef");
            else if (CFArrayGetCount((CFArrayRef)value) > 0)
            {
                // Recursively process children

                auto cf_children = (CFArrayRef)value;
                for (CFIndex i = 0, c = CFArrayGetCount(cf_children); i < c; ++i)
                {
                    value = CFArrayGetValueAtIndex(cf_children, i);
                    if (!value)
                        throw runtime_error("Fetched child is null");
                    else if (CFGetTypeID(value) == kAXValueAXErrorType)
                        throw runtime_error("Fetched child is kAXValueAXErrorType");
                    else if (CFGetTypeID(value) != AXUIElementGetTypeID())
                        throw runtime_error("Fetched child is not of type AXUIElementRef");

                    retrieveMenuItemsRecurse(valid, items, icon, path, (AXUIElementRef)value);
                }
            }
            else if (CFArrayRef actions = nullptr;
                     AXUIElementCopyActionNames(element, &actions) == kAXErrorSuccess && actions)
            {
                QString command_char;
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdChar);
                    v && CFGetTypeID(v) == CFStringGetTypeID())
                    command_char = QString::fromCFString((CFStringRef)v);

                // if there is a glyph use that instead
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdGlyph);
                    v && CFGetTypeID(v) == CFNumberGetTypeID())
                {
                    int glyphID = [(__bridge NSNumber*)v intValue];
                    if (auto it = glyph_map.find(glyphID); it != glyph_map.end())
                        command_char = it->second;
                }

                Qt::KeyboardModifiers mods;
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdModifiers);
                    v && CFGetTypeID(v) == CFNumberGetTypeID())
                    mods = toQt([(__bridge NSNumber*)v intValue]);

                QString shortcut;
                if (!command_char.isEmpty())
                    shortcut = QKeySequence(mods).toString(QKeySequence::NativeText) % command_char;

                if (CFArrayContainsValue(actions,
                                         CFRangeMake(0, CFArrayGetCount(actions)),
                                         kAXPressAction))
                    items.emplace_back(make_shared<MenuItem>(element, path, shortcut, icon.clone()));

                CFRelease(actions);
            }
        } catch (const skip &e) {
        } catch (const exception &e) {
            DEBG << e.what();
        }

        CFRelease(attribute_values);
    }
    CFRelease(attributes_array);
}

static vector<shared_ptr<MenuItem>> retrieveMenuBarItems(const bool &valid)
{
    vector<shared_ptr<MenuItem>> menu_items;
    auto app = NSWorkspace.sharedWorkspace.frontmostApplication;
    auto app_ax = AXUIElementCreateApplication(app.processIdentifier);
    const auto icon = makeFileTypeIcon(QUrl::fromNSURL(app.bundleURL).toLocalFile());

    CFTypeRef app_ax_menu_bar = nullptr;
    if (auto error = AXUIElementCopyAttributeValue(app_ax, kAXMenuBarAttribute, &app_ax_menu_bar);
        error != kAXErrorSuccess)
        WARN << u"Failed to retrieve menubar: %1 (See AXError.h)"_s.arg(error);
    else if (!app_ax_menu_bar)
        WARN << "Failed to retrieve menubar: Returned null.";
    else {
        CFTypeRef ax_menus = nullptr;
        if (error = AXUIElementCopyAttributeValue((AXUIElementRef)app_ax_menu_bar,
                                                  kAXChildrenAttribute,
                                                  &ax_menus);
            error != kAXErrorSuccess)
            WARN << u"Failed to retrieve menu bar menus: %1 (See AXError.h)"_s.arg(error);
        else if (!ax_menus)
            WARN << "Failed to retrieve menu bar menus: Returned null.";
        else {
            // Skip "Apple" menu
            for (CFIndex i = 1, c = CFArrayGetCount((CFArrayRef) ax_menus); i < c; ++i)
                retrieveMenuItemsRecurse(
                    valid, menu_items, *icon, {},
                    (AXUIElementRef)CFArrayGetValueAtIndex((CFArrayRef) ax_menus, i)
                );

            CFRelease(ax_menus);
        }
        CFRelease(app_ax_menu_bar);
    }
    CFRelease(app_ax);

    return menu_items;
}

class Plugin::Private
{
public:
    atomic_bool fuzzy;
    /*pid_t*/ atomic_int current_menu_pid = 0;
    shared_mutex items_mtx;
    vector<shared_ptr<MenuItem>> menu_items;
};

Plugin::Plugin() : d(make_unique<Private>())
{
    if (!AXIsProcessTrusted())
    {
        DEBG << "Accessibility permission denied.";
        QMessageBox::information(nullptr, {},
                                 tr("The menu bar plugin requires accessibility permissions to "
                                    "access the menu items of the focused application.\n\n"
                                    "macOS requires you to enable this manually in system "
                                    "settings. Please toggle Albert in the accessibility settings, "
                                    "which will appear after you close this dialog."));

        // Note: does not add an entry to the privacy settings in debug mode
        NSString* prefPage = @"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility";
        [[NSWorkspace sharedWorkspace] openURL:(NSURL * __nonnull)[NSURL URLWithString:prefPage]];
    }
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const { return u"m "_s; }

bool Plugin::supportsFuzzyMatching() const { return true; }

void Plugin::setFuzzyMatching(bool enabled) { d->fuzzy = enabled; }

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    if (!AXIsProcessTrusted())
    {
        WARN << "Accessibility permission denied.";
        return {};
    }

    // Update menu if app changed
    auto app = NSWorkspace.sharedWorkspace.frontmostApplication;
    if (app && d->current_menu_pid != (int)app.processIdentifier)
    {
        d->current_menu_pid = (int)app.processIdentifier;

        // AX api is not thread save, dispatch in main thread
        __block vector<shared_ptr<MenuItem>> menu_items;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_main_queue(), ^{
            menu_items = retrieveMenuBarItems(query.isValid());
            dispatch_semaphore_signal(semaphore);
        });
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);  // Wait for user

        scoped_lock lock(d->items_mtx);
        d->menu_items = ::move(menu_items);
    }

    vector<RankItem> results;
    Matcher matcher(query.string(), {.fuzzy = d->fuzzy});
    shared_lock lock(d->items_mtx);
    for (const auto& item : d->menu_items)
        if (auto m = matcher.match(item->text(), item->pathString()); m)
            results.emplace_back(item, m);

    return results;
}
