// Copyright (c) 2022-2025 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include "terminal.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QWidget>
#include <albert/logging.h>
#include <albert/systemutil.h>
#include <albert/widgetsutil.h>
#include <Windows.h>
#include <shlobj.h>
#include <map>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

static QStringList appDirectories()
{
    QStringList dirs;

    // Get Common Start Menu
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, path)))
        dirs << QString::fromWCharArray(path);

    // Get User Start Menu
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, 0, path)))
        dirs << QString::fromWCharArray(path);

    // Also scan WindowsApps directories for UWP/Store apps
    // WindowsApps can be in:
    // 1. %LOCALAPPDATA%\Microsoft\WindowsApps (user-specific apps)
    // 2. %ProgramFiles%\WindowsApps (system-wide apps like Calculator)
    QString localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    // QStandardPaths::GenericDataLocation returns AppData\Local on Windows
    QString windowsAppsPath = QDir(localAppData).absoluteFilePath(u"Microsoft/WindowsApps"_s);
    if (QDir(windowsAppsPath).exists())
    {
        dirs << windowsAppsPath;
        DEBG << "Added WindowsApps directory:" << windowsAppsPath;
    }
    
    // Also check Program Files WindowsApps (system-wide UWP apps)
    QString programFilesWindowsApps = u"C:/Program Files/WindowsApps"_s;
    if (QDir(programFilesWindowsApps).exists())
    {
        dirs << programFilesWindowsApps;
        DEBG << "Added Program Files WindowsApps directory:" << programFilesWindowsApps;
    }
    
    // Also try Program Files (x86) for 32-bit apps
    QString programFilesX86WindowsApps = u"C:/Program Files (x86)/WindowsApps"_s;
    if (QDir(programFilesX86WindowsApps).exists())
    {
        dirs << programFilesX86WindowsApps;
        DEBG << "Added Program Files (x86) WindowsApps directory:" << programFilesX86WindowsApps;
    }

    return dirs;
}

Plugin* plugin = nullptr;

Plugin::Plugin()
{
    plugin = this;

    fs_watcher.addPaths(appDirectories());
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);

    // Load settings
    auto s = settings();

    restore_ignore_show_in_keys(s);
    connect(this, &Plugin::ignore_show_in_keys_changed,
            this, &Plugin::updateIndexItems);

    restore_use_exec(s);
    connect(this, &Plugin::use_exec_changed,
            this, &Plugin::updateIndexItems);

    restore_use_generic_name(s);
    connect(this, &Plugin::use_generic_name_changed,
            this, &Plugin::updateIndexItems);

    restore_use_keywords(s);
    connect(this, &Plugin::use_keywords_changed,
            this, &Plugin::updateIndexItems);

    restore_use_non_localized_name(s);
    connect(this, &PluginBase::use_non_localized_name_changed,
            this, &Plugin::updateIndexItems);

    // File watches
    for (const auto &path : appDirectories())
        for (auto dit = QDirIterator(path, QDir::Dirs|QDir::NoDotDot, QDirIterator::Subdirectories); dit.hasNext();)
            fs_watcher.addPath(QFileInfo(dit.next()).canonicalFilePath());

    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](){ indexer.run(); });

    // Indexer setup - scan Windows Start Menu shortcuts
    indexer.parallel = [this](const bool &abort) -> vector<shared_ptr<applications::Application>>
    {
        Application::ParseOptions po{
            .ignore_show_in_keys = ignore_show_in_keys(),
            .use_exec = use_exec(),
            .use_generic_name = use_generic_name(),
            .use_keywords = use_keywords(),
            .use_non_localized_name = use_non_localized_name()
        };

        vector<shared_ptr<applications::Application>> apps;
        
        // Scan Start Menu directories for .lnk files and WindowsApps for .exe files
        for (const QString &dir : appDirectories())
        {
            if (abort)
                return apps;

            DEBG << "Scanning Windows applications in:" << dir;

            // Check if this is the WindowsApps directory (for UWP apps)
            bool isWindowsApps = dir.contains(u"WindowsApps"_s, Qt::CaseInsensitive);
            
            QStringList filters;
            QMap<QString, QStringList> validExes; // For WindowsApps: package dir -> list of valid exe names
            
            if (isWindowsApps)
            {
                // For WindowsApps, we need to filter to only main app executables
                // Parse AppxManifest.xml files to find registered applications
                QDirIterator manifestIt(dir, QStringList() << u"AppxManifest.xml"_s, 
                                       QDir::Files, QDirIterator::Subdirectories);
                
                while (manifestIt.hasNext())
                {
                    QString manifestPath = manifestIt.next();
                    QFile manifestFile(manifestPath);
                    if (manifestFile.open(QIODevice::ReadOnly | QIODevice::Text))
                    {
                        QByteArray data = manifestFile.readAll();
                        QString content = QString::fromUtf8(data);
                        
                        // Extract Executable attribute from Application elements
                        QRegularExpression exeRegex(uR"(<Application[^>]*Executable=["']([^"']+)["'])"_s);
                        QRegularExpressionMatchIterator matches = exeRegex.globalMatch(content);
                        
                        QStringList exeList;
                        while (matches.hasNext())
                        {
                            QRegularExpressionMatch match = matches.next();
                            QString exePath = match.captured(1);
                            // Convert to just filename for easier matching
                            exeList << QFileInfo(exePath).fileName();
                        }
                        
                        if (!exeList.isEmpty())
                        {
                            QString packageDir = QFileInfo(manifestPath).dir().absolutePath();
                            validExes[packageDir] = exeList;
                        }
                    }
                }
                
                // In WindowsApps, scan for .exe files in subdirectories only
                filters << u"*.exe"_s;
            }
            else
            {
                // In Start Menu, scan for .lnk shortcuts
                filters << u"*.lnk"_s;
            }

            QDirIterator it(dir, filters, QDir::Files,
                            QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

            // Track canonical paths to avoid duplicates (symlinks vs actual files)
            map<QString, QString> canonicalPaths;  // canonical path -> original path

            while (it.hasNext())
            {
                if (abort)
                    return apps;

                auto path = it.next();
                QFileInfo fi(path);
                
                // For WindowsApps, skip root-level files and symlinks from LocalAppData
                if (isWindowsApps)
                {
                    QString relativePath = QDir(dir).relativeFilePath(path);
                    // Skip if file is directly in WindowsApps root (contains no subdirectory)
                    if (!relativePath.contains(u'/') && !relativePath.contains(u'\\'))
                    {
                        DEBG << QStringLiteral("Skipping root-level symlink: '%1'").arg(path);
                        continue;
                    }
                    
                    // Skip LocalAppData WindowsApps entries (they're symlinks to Program Files apps)
                    // We only want the real apps from Program Files\WindowsApps
                    if (dir.contains(u"Local"_s, Qt::CaseInsensitive) && 
                        dir.contains(u"AppData"_s, Qt::CaseInsensitive))
                    {
                        DEBG << QStringLiteral("Skipping LocalAppData symlink: '%1'").arg(path);
                        continue;
                    }
                    
                    // If this package has a manifest, only include executables registered in it
                    // If no manifest exists, include the exe (it's a standalone app)
                    QString packageDir = fi.dir().absolutePath();
                    if (validExes.contains(packageDir))
                    {
                        QString exeName = fi.fileName();
                        if (!validExes[packageDir].contains(exeName, Qt::CaseInsensitive))
                        {
                            DEBG << QStringLiteral("Skipping non-registered executable: '%1'").arg(path);
                            continue;
                        }
                    }
                    // If no manifest found, allow the exe through (apps without manifests are valid)
                }
                
                // Get canonical path to detect duplicates (symlinks pointing to same file)
                QString canonicalPath = fi.canonicalFilePath();
                if (canonicalPath.isEmpty())
                    canonicalPath = path;  // Fallback if canonical path fails
                
                // Check if we've already seen this canonical path
                if (canonicalPaths.find(canonicalPath) != canonicalPaths.end())
                {
                    DEBG << u"Skipping duplicate (same canonical path): '%1' (already have '%2')"_s
                                .arg(path, canonicalPaths[canonicalPath]);
                    continue;
                }
                
                canonicalPaths[canonicalPath] = path;
                
                // Use the filename (without extension) as the ID
                QString id = fi.completeBaseName();

                try
                {
                    apps.emplace_back(make_shared<Application>(id, path, po));
                    DEBG << u"Found application '%1': '%2'"_s.arg(id, path);
                }
                catch (const exception &e)
                {
                    DEBG << u"Skipped application '%1':"_s.arg(path) << e.what();
                }
            }
        }

        return apps;
    };

    indexer.finish = [this](vector<shared_ptr<applications::Application>> &&result)
    {
        applications = ::move(result);

        INFO << u"Indexed %1 applications [%2 ms]"_s
                    .arg(applications.size()).arg(indexer.runtime.count());

        // Build terminals list (Windows doesn't have terminal detection like Unix)
        // For now, terminals list will be empty, which is fine
        terminals.clear();
        setUserTerminalFromConfig();

        setIndexItems(buildIndexItems());

        emit appsChanged();
    };

    // Build terminals list
    setUserTerminalFromConfig();

    // Populate index
    commonInitialize(s);

    indexer.run();
}

Plugin::~Plugin()
{
    plugin = nullptr;
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    bind(ui.checkBox_ignoreShowInKeys,
         this,
         &Plugin::ignore_show_in_keys,
         &Plugin::set_ignore_show_in_keys);

    bind(ui.checkBox_useExec,
         this,
         &Plugin::use_exec,
         &Plugin::set_use_exec);

    bind(ui.checkBox_useGenericName,
         this,
         &Plugin::use_generic_name,
         &Plugin::set_use_generic_name);

    bind(ui.checkBox_useKeywords,
         this,
         &Plugin::use_keywords,
         &Plugin::set_use_keywords);

    addBaseConfig(ui.formLayout);

    return w;
}

QJsonObject Plugin::telemetryData() const
{
    QJsonObject data;
    data[QStringLiteral("app_count")] = (int)applications.size();
    return data;
}

void Plugin::runTerminal(QStringList commandline, const QString working_dir) const
{
    if (terminal)
        terminal->launch(commandline, working_dir);
    else
    {
        // Fallback: use cmd.exe
        QStringList cmd;
        cmd << u"cmd.exe"_s << u"/K"_s << commandline.join(u" "_s);
        runDetachedProcess(cmd, working_dir);
    }
}
