// Copyright (c) 2022-2025 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include <utility>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QPixmap>
#include <QIcon>
#include <QImage>
#include <QRegularExpression>
#include <QIODevice>
#include <albert/iconutil.h>
#include <albert/systemutil.h>
#include <Windows.h>
#include <ShlObj.h>
#include <shlwapi.h>
#include <wingdi.h>
#include <comdef.h>
#include <comip.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shellapi.h>
#include <shobjidl.h>
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace albert::util;
using namespace albert;
using namespace std;

extern Plugin* plugin;

Application::Application(const QString &id, const QString &path, ParseOptions po)
{
    id_ = id;
    path_ = path;

    // Initialize COM for Windows Shell API
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Parse Windows shortcut (.lnk) file using IShellLink
    QString name = id; // Fallback to ID
    QString targetPath;
    QString description;
    
    if (path.endsWith(u".lnk"_s, Qt::CaseInsensitive))
    {
        // Use Windows Shell API to parse .lnk file
        _COM_SMARTPTR_TYPEDEF(IShellLink, IID_IShellLink);
        _COM_SMARTPTR_TYPEDEF(IPersistFile, IID_IPersistFile);
        _COM_SMARTPTR_TYPEDEF(IPropertyStore, IID_IPropertyStore);
        
        IShellLinkPtr psl;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        
        if (SUCCEEDED(hr))
        {
            IPersistFilePtr ppf(psl);
            hr = ppf->Load(path.toStdWString().c_str(), STGM_READ);
            
            if (SUCCEEDED(hr))
            {
                // Get the target path
                wchar_t szTarget[MAX_PATH];
                hr = psl->GetPath(szTarget, MAX_PATH, NULL, SLGP_RAWPATH);
                if (FAILED(hr))
                {
                    // Try without RAWPATH flag
                    hr = psl->GetPath(szTarget, MAX_PATH, NULL, 0);
                }
                if (SUCCEEDED(hr))
                    targetPath = QString::fromWCharArray(szTarget);
                
                // Get the description
                wchar_t szDesc[MAX_PATH];
                hr = psl->GetDescription(szDesc, MAX_PATH);
                if (SUCCEEDED(hr))
                    description = QString::fromWCharArray(szDesc);
                
                // Try to get the display name from IPropertyStore
                IPropertyStorePtr pps(psl);
                if (pps)
                {
                    PROPVARIANT pv;
                    PropVariantInit(&pv);
                    // Try PKEY_Title for display name first
                    hr = pps->GetValue(PKEY_Title, &pv);
                    if (SUCCEEDED(hr) && pv.vt == VT_LPWSTR)
                    {
                        name = QString::fromWCharArray(pv.pwszVal);
                        PropVariantClear(&pv);
                    }
                    else
                    {
                        PropVariantClear(&pv);
                    }
                }
                
                // If we didn't get a name from properties, use the shortcut filename
                if (name == id)
                {
                    QFileInfo fi(path);
                    name = fi.completeBaseName();
                }
            }
        }
    }
    else
    {
        // Not a .lnk file, use filename
        QFileInfo fi(path);
        name = fi.completeBaseName();
        targetPath = path;
    }

    // Use the extracted name
    names_ << name;
    
    // Always add the executable name for searchability (e.g., "notepad" from "notepad.exe")
    // This ensures users can search by executable name even if shortcut has different name
    if (!targetPath.isEmpty())
    {
        QFileInfo targetFi(targetPath);
        QString exeName = targetFi.completeBaseName();
        if (!exeName.isEmpty())
        {
            // Check if exeName is already in names_ (case-insensitive)
            bool found = false;
            for (const QString &n : names_)
            {
                if (n.compare(exeName, Qt::CaseInsensitive) == 0)
                {
                    found = true;
                    break;
                }
            }
            // Add executable name if not already present
            if (!found)
                names_ << exeName;
        }
    }
    else if (!path.isEmpty() && !path.endsWith(u".lnk"_s, Qt::CaseInsensitive))
    {
        // For non-.lnk files, also add the base name
        QFileInfo fi(path);
        QString baseName = fi.completeBaseName();
        if (!baseName.isEmpty())
        {
            bool found = false;
            for (const QString &n : names_)
            {
                if (n.compare(baseName, Qt::CaseInsensitive) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                names_ << baseName;
        }
    }

    if (po.use_non_localized_name)
        names_ << name;

    // Set the executable path
    if (!targetPath.isEmpty())
        exec_ << targetPath;
    else
        exec_ << path;

    // Set description
    description_ = description.isEmpty() ? targetPath : description;

    // Try to get an icon from the target executable
    // Use canonical path for better icon extraction (especially for UWP apps)
    QString iconPath = targetPath.isEmpty() ? path : targetPath;
    QFileInfo iconFi(iconPath);
    QString canonicalIconPath = iconFi.canonicalFilePath();
    icon_ = canonicalIconPath.isEmpty() ? iconPath : canonicalIconPath;

    names_.removeDuplicates();
}

QString Application::subtext() const { return description_; }

unique_ptr<Icon> Application::icon() const
{
    // Use makeFileTypeIcon which handles Windows file icons properly
    // It uses QFileIconProvider internally which works well with Windows shortcuts and executables
    return makeFileTypeIcon(icon_);
}

vector<Action> Application::actions() const
{
    vector<Action> actions = ApplicationBase::actions();

    actions.emplace_back(u"reveal-entry"_s,
                         Plugin::tr("Open shortcut location"),
                         [this] { open(QFileInfo(path_).dir().path()); });

    return actions;
}

const QStringList &Application::exec() const
{
    return exec_;
}

bool Application::isTerminal() const { return is_terminal_; }

void Application::launchExec(const QStringList &exec, QUrl url, const QString &working_dir) const
{
    Q_UNUSED(url)

    // On Windows, launch the shortcut directly using the Shell
    if (path_.endsWith(u".lnk"_s, Qt::CaseInsensitive))
    {
        // Use QProcess or Windows API to launch the shortcut
        runDetachedProcess(QStringList() << u"cmd.exe"_s << u"/c"_s << u"start"_s << u""_s << path_, working_dir);
    }
    else if (path_.contains(u"WindowsApps"_s, Qt::CaseInsensitive))
    {
        // UWP apps cannot be launched directly via their .exe - use IApplicationActivationManager
        // Extract Package Family Name and Application Id from the directory structure
        QFileInfo exeInfo(path_);
        QDir appDir = exeInfo.dir();
        QString dirName = appDir.dirName();
        
        // Package Family Name format: PackageName_PublisherIdHash
        // Directory format: PackageName_Version_Arch_PublisherIdHash
        // Extract first and last components
        QString packageFamilyName;
        QRegularExpression pfnRegex(uR"(^([^_]+)_.*_([^_]+)$)"_s);
        QRegularExpressionMatch pfnMatch = pfnRegex.match(dirName);
        if (pfnMatch.hasMatch())
        {
            QString packageName = pfnMatch.captured(1);
            QString publisherId = pfnMatch.captured(2);
            packageFamilyName = packageName + u"_"_s + publisherId;
        }
        
        QString aumid;
        QString manifestPath = appDir.absoluteFilePath(u"AppxManifest.xml"_s);
        if (!packageFamilyName.isEmpty() && QFileInfo(manifestPath).exists())
        {
            // Try to read the manifest and extract Application Id
            QFile manifestFile(manifestPath);
            if (manifestFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QByteArray data = manifestFile.readAll();
                QString content = QString::fromUtf8(data);
                
                // Extract Application Id from manifest
                QRegularExpression appIdRegex(uR"(<Application[^>]*Id=["']([^"']*)["'])"_s);
                QRegularExpressionMatch appIdMatch = appIdRegex.match(content);
                
                QString appId;
                if (appIdMatch.hasMatch())
                    appId = appIdMatch.captured(1);
                
                // AUMID format: PackageFamilyName!AppId
                aumid = packageFamilyName + u"!"_s + appId;
            }
        }
        
        if (!aumid.isEmpty())
        {
            // Try using IApplicationActivationManager first
            HRESULT comInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            bool needsUninit = (comInit == S_OK);
            
            IApplicationActivationManager* pActivationManager = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_INPROC_SERVER,
                                         IID_IApplicationActivationManager, (LPVOID*)&pActivationManager);
            
            if (SUCCEEDED(hr) && pActivationManager)
            {
                DWORD pid = 0;
                std::wstring waumid = aumid.toStdWString();
                hr = pActivationManager->ActivateApplication(waumid.c_str(), NULL, AO_NONE, &pid);
                pActivationManager->Release();
                
                if (SUCCEEDED(hr))
                {
                    if (needsUninit)
                        CoUninitialize();
                    return;
                }
            }
            
            if (needsUninit)
                CoUninitialize();
            
            // Fallback: use shell:AppsFolder protocol
            QString protocolUrl = u"shell:AppsFolder\\"_s + aumid;
            std::wstring wurl = protocolUrl.toStdWString();
            HINSTANCE result = ShellExecuteW(NULL, L"open", wurl.c_str(), NULL, NULL, SW_SHOWNORMAL);
            if (reinterpret_cast<intptr_t>(result) > 32)
            {
                return; // Successfully launched via protocol
            }
        }
        
        // Fallback: use start command with proper path handling
        // Convert forward slashes to backslashes for Windows
        QString winPath = path_;
        winPath.replace(u'/', u'\\');
        // Use start command - the empty string is required for window title
        runDetachedProcess(QStringList() << u"cmd.exe"_s << u"/c"_s << u"start"_s << u""_s << winPath, QString());
    }
    else
    {
        runDetachedProcess(exec, working_dir);
    }
}

void Application::launch() const { launchExec(exec_, {}, working_dir_); }
