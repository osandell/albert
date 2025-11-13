// Copyright (c) 2022-2025 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include <utility>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QPixmap>
#include <QIcon>
#include <QImage>
#include <albert/iconutil.h>
#include <albert/systemutil.h>
#include <albert/icons.h>
#include <Windows.h>
#include <ShlObj.h>
#include <shlwapi.h>
#include <wingdi.h>
#include <comdef.h>
#include <comip.h>
#include <propkey.h>
#include <propvarutil.h>
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
    // For UWP apps and Windows executables, use Windows Shell API for better icon extraction
    QIcon winIcon;
    
    if (!icon_.isEmpty())
    {
        // Use SHGetFileInfo to extract icon from executable/shortcut
        SHFILEINFOW sfi = {0};
        std::wstring wpath = icon_.toStdWString();
        DWORD_PTR result = SHGetFileInfoW(wpath.c_str(), 0, &sfi, sizeof(sfi), 
                                          SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
        
        if (result && sfi.hIcon)
        {
            // Convert HICON to QPixmap by drawing to a bitmap
            const int iconSize = 32; // Standard icon size
            HDC hdc = CreateCompatibleDC(NULL);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmp = CreateCompatibleBitmap(hdc, iconSize, iconSize);
            HGDIOBJ oldBmp = SelectObject(hdcMem, hbmp);
            
            // Draw icon to bitmap
            DrawIconEx(hdcMem, 0, 0, sfi.hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
            
            // Get bitmap bits
            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = iconSize;
            bmi.bmiHeader.biHeight = -iconSize; // Negative for top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            
            QImage img(iconSize, iconSize, QImage::Format_ARGB32);
            GetDIBits(hdcMem, hbmp, 0, iconSize, img.bits(), &bmi, DIB_RGB_COLORS);
            
            // Cleanup
            SelectObject(hdcMem, oldBmp);
            DeleteObject(hbmp);
            DeleteDC(hdcMem);
            DeleteDC(hdc);
            DestroyIcon(sfi.hIcon);
            
            winIcon = QIcon(QPixmap::fromImage(img));
        }
    }
    
    // Fallback to QFileIconProvider if Shell API failed
    if (winIcon.isNull())
    {
        return makeFileTypeIcon(icon_);
    }
    
    // Return QIconIcon wrapper
    return make_unique<QIconIcon>(winIcon);
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
    else
    {
        runDetachedProcess(exec, working_dir);
    }
}

void Application::launch() const { launchExec(exec_, {}, working_dir_); }
