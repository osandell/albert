// Copyright (c) 2022-2025 Manuel Schneider

#include "docitem.h"
#include "docset.h"
#include "plugin.h"
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/networkutil.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;


DocItem::DocItem(const Docset &ds, const QString &t, const QString &n, const QString &p, const QString &a)
    : docset(ds), type(t), name(n), path(p), anchor(a) {}

QString DocItem::id() const { return docset.name + name; }

QString DocItem::text() const { return name; }

QString DocItem::subtext() const { return u"%1 %2"_s.arg(docset.title, type); }

unique_ptr<Icon> DocItem::icon() const
{
    struct CustomEngine : public Icon
    {
        unique_ptr<Icon> icon_;

        CustomEngine(unique_ptr<Icon> e) : icon_(::move(e)) { }

        void paint(QPainter *p, const QRect &rect) override
        {
            const auto size = icon_->actualSize(rect.size(), p->device()->devicePixelRatio());
            const auto src_extent = max(size.width(), size.height());
            const auto dst_extent = min(rect.width(), rect.height());

            if (src_extent > dst_extent/2)
                icon_->paint(p, rect);
            else
                makeComposedIcon(makeGraphemeIcon(u"📖"_s), icon_->clone(), 1.0, 1.0)->paint(p, rect);
        }

        bool isNull() override { return icon_->isNull(); }

        unique_ptr<Icon> clone() const override { return make_unique<CustomEngine>(icon_->clone()); }

        QString toUrl() const override { return u"docs:"_s + icon_->toUrl(); }
    };

    return make_unique<CustomEngine>(makeImageIcon(docset.icon_path));
}

QString DocItem::inputActionText() const { return name; }

vector<Action> DocItem::actions() const
{ return {{ id(), Plugin::tr("Open documentation"), [this] { open(); } }}; }

// Workaround for some browsers not opening "file:" urls having an anchor
void DocItem::open() const
{
    // QTemporaryFile will not work here because its deletion introduces race condition
    const auto cache = Plugin::instance()->cacheLocation();
    util::tryCreateDirectory(cache);
    if (QFile file(cache / "trampoline.html");
        file.open(QIODevice::WriteOnly))
    {
        auto url = u"file:%1/Contents/Resources/Documents/%2"_s.arg(docset.path, path);
        if (!anchor.isEmpty())
            url += u"#"_s + anchor;

        QTextStream stream(&file);
        stream << uR"(<html><head><meta http-equiv="refresh" content="0;%1"></head></html>)"_s
                      .arg(url);
        file.close();

        util::open(file.fileName());
    }
    else
        WARN << "Failed to open file for writing" << file.fileName() << file.errorString();
}

