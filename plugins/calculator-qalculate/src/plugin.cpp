// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QSettings>
#include <QThread>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
ALBERT_LOGGING_CATEGORY("qalculate")
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

namespace {
const auto URL_MANUAL      = u"https://qalculate.github.io/manual/index.html"_s;
const auto CFG_ANGLEUNIT   = u"angle_unit"_s;
const auto DEF_ANGLEUNIT   = (int)ANGLE_UNIT_RADIANS;
const auto CFG_PARSINGMODE = "parsing_mode";
const auto DEF_PARSINGMODE = (int)PARSING_MODE_CONVENTIONAL;
const auto CFG_PRECISION   = u"precision"_s;
const auto DEF_PRECISION   = 16;
const auto CFG_UNITS       = u"units_in_global_query"_s;
const auto DEF_UNITS       = false;
const auto CFG_FUNCS       = u"functions_in_global_query"_s;
const auto DEF_FUNCS       = false;

static unique_ptr<Icon> makeIcon() { return makeThemeIcon(u"accessories-calculator"_s); }
}

Plugin::Plugin()
{
    auto s = settings();

    // init calculator
    qalc.reset(new Calculator());
    qalc->loadExchangeRates();
    qalc->loadGlobalCurrencies();
    qalc->loadGlobalDefinitions();
    qalc->loadLocalDefinitions();
    qalc->setPrecision(s->value(CFG_PRECISION, DEF_PRECISION).toInt());

    // evaluation options
    eo.auto_post_conversion = POST_CONVERSION_BEST;
    eo.structuring = STRUCTURING_SIMPLIFY;

    // parse options
    eo.parse_options.angle_unit = static_cast<AngleUnit>(s->value(CFG_ANGLEUNIT, DEF_ANGLEUNIT).toInt());
    eo.parse_options.functions_enabled = s->value(CFG_FUNCS, DEF_FUNCS).toBool();
    eo.parse_options.limit_implicit_multiplication = true;
    eo.parse_options.parsing_mode = static_cast<ParsingMode>(s->value(CFG_PARSINGMODE, DEF_PARSINGMODE).toInt());
    eo.parse_options.units_enabled = s->value(CFG_UNITS, DEF_UNITS).toBool();
    eo.parse_options.unknowns_enabled = false;

    // print options
    po.indicate_infinite_series = true;
    po.interval_display = INTERVAL_DISPLAY_SIGNIFICANT_DIGITS;
    po.lower_case_e = true;
    //po.preserve_precision = true;  // https://github.com/albertlauncher/plugins/issues/92
    po.use_unicode_signs = true;
    //po.abbreviate_names = true;
}

QString Plugin::defaultTrigger() const { return u"="_s; }

QString Plugin::synopsis(const QString &) const
{
    static const auto tr_me = tr("<math expression>");
    return tr_me;
}

QWidget *Plugin::buildConfigWidget()
{
    auto *widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    // Angle unit
    ui.angleUnitComboBox->setCurrentIndex(eo.parse_options.angle_unit);
    connect(ui.angleUnitComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        settings()->setValue(CFG_ANGLEUNIT, index);
        lock_guard locker(qalculate_mutex);
        eo.parse_options.angle_unit = static_cast<AngleUnit>(index);
    });

    // Parsing mode
    ui.parsingModeComboBox->setCurrentIndex(eo.parse_options.parsing_mode);
    connect(ui.parsingModeComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        settings()->setValue(CFG_PARSINGMODE, index);
        lock_guard locker(qalculate_mutex);
        eo.parse_options.parsing_mode = static_cast<ParsingMode>(index);
    });

    // Precision
    ui.precisionSpinBox->setValue(qalc->getPrecision());
    connect(ui.precisionSpinBox,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [this](int value){
        settings()->setValue(CFG_PRECISION, value);
        lock_guard locker(qalculate_mutex);
        qalc->setPrecision(value);
    });

    // Units in global query
    ui.unitsInGlobalQueryCheckBox->setChecked(eo.parse_options.units_enabled);
    connect(ui.unitsInGlobalQueryCheckBox, &QCheckBox::toggled, this, [this](bool checked)
    {
        settings()->setValue(CFG_UNITS, checked);
        lock_guard locker(qalculate_mutex);
        eo.parse_options.units_enabled = checked;
    });

    // Functions in global query
    ui.functionsInGlobalQueryCheckBox->setChecked(eo.parse_options.functions_enabled);
    connect(ui.functionsInGlobalQueryCheckBox, &QCheckBox::toggled, this, [this](bool checked)
    {
        settings()->setValue(CFG_FUNCS, checked);
        lock_guard locker(qalculate_mutex);
        eo.parse_options.functions_enabled = checked;
    });

    return widget;
}

shared_ptr<Item> Plugin::buildItem(const QString &query, MathStructure &mstruct) const
{
    static const auto tr_tr = tr("Copy result to clipboard");
    static const auto tr_te = tr("Copy equation to clipboard");
    static const auto tr_e = tr("Result of %1");
    static const auto tr_a = tr("Approximate result of %1");

    mstruct.format(po);
    auto result = QString::fromStdString(mstruct.print(po));

    return StandardItem::make(
        u"qalc-res"_s,
        result,
        mstruct.isApproximate() ? tr_a.arg(query) : tr_e.arg(query),
        makeIcon,
        {
            {u"cpr"_s, tr_tr, [=](){ setClipboardText(result); }},
            {u"cpe"_s, tr_te, [=](){ setClipboardText(QString(u"%1 = %2"_s).arg(query, result)); }}
        }
    );
}

variant<QStringList, MathStructure> Plugin::runQalculateLocked(const Query &query,
                                                               const EvaluationOptions &eo_)
{
    MathStructure mstruct;
    auto expression = qalc->unlocalizeExpression(query.string().toStdString(), eo.parse_options);

    qalc->startControl();
    qalc->calculate(&mstruct, expression, 0, eo_);
    for (; qalc->busy(); QThread::msleep(10))
        if (!query.isValid())
            qalc->abort();
    qalc->stopControl();

    if (qalc->message())
    {
        QStringList errors;
        for (auto msg = qalc->message(); msg; msg = qalc->nextMessage())
            errors << QString::fromUtf8(qalc->message()->c_message());
        return errors;
    }
    else
        return mstruct;
}

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> results;

    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        return results;

    lock_guard locker(qalculate_mutex);

    auto var = runQalculateLocked(query, eo);

    if (query.isValid() && holds_alternative<MathStructure>(var))
        results.emplace_back(buildItem(trimmed, get<MathStructure>(var)), 1.0f);

    return results;
}

void Plugin::handleTriggerQuery(Query &query)
{
    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        return;

    auto eo_ = eo;
    eo_.parse_options.functions_enabled = true;
    eo_.parse_options.units_enabled = true;
    eo_.parse_options.unknowns_enabled = true;

    lock_guard locker(qalculate_mutex);
    auto var = runQalculateLocked(query, eo_);

    if (!query.isValid())
        return;

    if (holds_alternative<MathStructure>(var))
        query.add(buildItem(trimmed, get<MathStructure>(var)));

    else
    {
        static const auto tr_e = tr("Evaluation error.");
        static const auto tr_d = tr("Visit documentation");
        query.add(
            StandardItem::make(
                u"qalc-err"_s,
                tr_e,
                get<QStringList>(var).join(u", "_s),
                makeIcon,
                {{u"manual"_s, tr_d, [=](){ openUrl(URL_MANUAL); }}},
                u""_s
            )
        );
    }
}
