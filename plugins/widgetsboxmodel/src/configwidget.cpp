// Copyright (c) 2022-2025 Manuel Schneider

#include "configwidget.h"
#include "ui_configwidget.h"
#include "window.h"
#include <QGroupBox>
#include <albert/widgetsutil.h>
using namespace albert;
using namespace std;
using namespace util;

template<typename T>
static QSpinBox *createSpinBox(QFormLayout *form_layout,
                               QString label,
                               T *t,
                               uint (T::*get)() const,
                               void (T::*set)(uint))
{
    auto *spin_box = new QSpinBox;
    spin_box->setValue((t->*get)());
    QObject::connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), t, set);
    spin_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    form_layout->addRow(label, spin_box);
    return spin_box;
}

template<typename T>
static auto addFontSpinBox(QFormLayout *form_layout,
                           QString label,
                           T *t,
                           uint (T::*get)() const,
                           void (T::*set)(uint))
{
    auto *spin_box = createSpinBox(form_layout, label, t, get, set);
    spin_box->setMinimum(6);
    spin_box->setSuffix(QStringLiteral(" pt"));
    return spin_box;
}

template<typename T>
static auto addPixelMetricSpinBox(QFormLayout *form_layout,
                                  QString label,
                                  T *t,
                                  uint (T::*get)() const,
                                  void (T::*set)(uint))
{
    auto *spin_box = createSpinBox(form_layout, label, t, get, set);
    spin_box->setSuffix(QStringLiteral(" px"));
    return spin_box;
}

template<typename T>
static auto addPixelMetricSpinBox(QFormLayout *form_layout,
                                  QString label,
                                  T *t,
                                  double (T::*get)() const,
                                  void (T::*set)(double))
{

    auto *spin_box = new QDoubleSpinBox;
    spin_box->setValue((t->*get)());
    spin_box->setSingleStep(0.5);
    spin_box->setDecimals(1);
    spin_box->setSuffix(QStringLiteral(" px"));
    spin_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(spin_box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), t, set);
    form_layout->addRow(label, spin_box);
    return spin_box;
}


ConfigWidget::ConfigWidget(Window &_window):
    window(_window)
{
    Ui::ConfigWidget ui;
    ui.setupUi(this);

    ui.comboBox_theme_light->addItem(tr("System"), QString());
    ui.comboBox_theme_light->insertSeparator(1);
    for (const auto&[name, _] : window.themes)
    {
        ui.comboBox_theme_light->addItem(name, name);
        if (name == window.themeLight())
            ui.comboBox_theme_light->setCurrentIndex(ui.comboBox_theme_light->count()-1);
    }
    connect(ui.comboBox_theme_light,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_light](int i)
            { window.setThemeLight(comboBox_themes->itemData(i).toString()); });

    connect(&window, &Window::themeLightChanged, this, [cb=ui.comboBox_theme_light](QString theme){
        if (auto i = cb->findData(theme); i != -1)
            cb->setCurrentIndex(i);
    });


    ui.comboBox_theme_dark->addItem(tr("System"), QString());
    ui.comboBox_theme_dark->insertSeparator(1);
    for (const auto&[name, _] : window.themes)
    {
        ui.comboBox_theme_dark->addItem(name, name);
        if (name == window.themeDark())
            ui.comboBox_theme_dark->setCurrentIndex(ui.comboBox_theme_dark->count()-1);
    }
    connect(ui.comboBox_theme_dark,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_dark](int i)
            { window.setThemeDark(comboBox_themes->itemData(i).toString()); });
    connect(&window, &Window::themeDarkChanged, this, [cb=ui.comboBox_theme_dark](QString theme){
        if (auto i = cb->findData(theme); i != -1)
            cb->setCurrentIndex(i);
    });

    bind(ui.checkBox_onTop,
         &window,
         &Window::alwaysOnTop,
         &Window::setAlwaysOnTop,
         &Window::alwaysOnTopChanged);

    bind(ui.checkBox_clearOnHide,
         &window,
         &Window::clearOnHide,
         &Window::setClearOnHide,
         &Window::clearOnHideChanged);

    bind(ui.checkBox_scrollbar,
         &window,
         &Window::displayScrollbar,
         &Window::setDisplayScrollbar,
         &Window::displayScrollbarChanged);

    bind(ui.checkBox_followCursor,
         &window,
         &Window::followCursor,
         &Window::setFollowCursor,
         &Window::followCursorChanged);

    bind(ui.checkBox_hideOnFocusOut,
         &window,
         &Window::hideOnFocusLoss,
         &Window::setHideOnFocusLoss,
         &Window::hideOnFocusLossChanged);

    bind(ui.checkBox_history_search,
         &window,
         &Window::historySearchEnabled,
         &Window::setHistorySearchEnabled,
         &Window::historySearchEnabledChanged);

    ui.spinBox_results->setValue((int)window.maxResults());
    connect(ui.spinBox_results, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            &window, &Window::setMaxResults);
    connect(&window, &Window::maxResultsChanged,
            ui.spinBox_results, &QSpinBox::setValue);

    bind(ui.checkBox_quit_on_close,
         &window,
         &Window::quitOnClose,
         &Window::setQuitOnClose,
         &Window::quitOnCloseChanged);

    bind(ui.checkBox_input_method,
         &window,
         &Window::disableInputMethod,
         &Window::setDisableInputMethod);

    bind(ui.checkBox_center,
         &window,
         &Window::showCentered,
         &Window::setShowCentered,
         &Window::showCenteredChanged);

    bind(ui.checkBox_debug,
         &window,
         &Window::debugMode,
         &Window::setDebugMode,
         &Window::debugModeChanged);

    connect(&window, &Window::debugModeChanged,
            ui.pushButton_winprop, &QPushButton::setEnabled);

    connect(ui.pushButton_winprop, &QPushButton::pressed, this, [this]
    {
        auto w = new QWidget;
        auto wl = new QVBoxLayout(w);

        auto b = new QGroupBox(tr("Window"));
        auto bl = new QFormLayout(b);

        addPixelMetricSpinBox(bl, tr("Shadow size"), &window,
                              &Window::windowShadowSize, &Window::setWindowShadowSize);

        addPixelMetricSpinBox(bl, tr("Shadow offset"), &window,
                              &Window::windowShadowOffset, &Window::setWindowShadowOffset);


        auto sb = addPixelMetricSpinBox(bl, tr("Width"), &window,
                                        &Window::windowWidth, &Window::setWindowWidth);
        sb->setSingleStep(10);
        QSignalBlocker block(sb);  // setRange emits value change
        sb->setRange(100, 9999);

        addPixelMetricSpinBox(bl, tr("Border radius"), &window,
                              &Window::windowBorderRadius, &Window::setWindowBorderRadius);

        addPixelMetricSpinBox(bl, tr("Border width"), &window,
                              &Window::windowBorderWidth, &Window::setWindowBorderWidth);

        addPixelMetricSpinBox(bl, tr("Padding"), &window,
                              &Window::windowPadding, &Window::setWindowPadding);

        addPixelMetricSpinBox(bl, tr("Spacing"), &window,
                              &Window::windowSpacing, &Window::setWindowSpacing);

        wl->addWidget(b);
        b = new QGroupBox(tr("Input"));
        bl = new QFormLayout(b);

        addFontSpinBox(bl, tr("Font size"), &window,
                       &Window::inputFontSize, &Window::setInputFontSize);

        addPixelMetricSpinBox(bl, tr("Border radius"), &window,
                              &Window::inputBorderRadius, &Window::setInputBorderRadius);

        addPixelMetricSpinBox(bl, tr("Border width"), &window,
                              &Window::inputBorderWidth, &Window::setInputBorderWidth);

        addPixelMetricSpinBox(bl, tr("Padding"), &window,
                              &Window::inputPadding, &Window::setInputPadding);

        wl->addWidget(b);
        b = new QGroupBox(tr("Results"));
        bl = new QFormLayout(b);

        addFontSpinBox(bl, tr("Font size"), &window,
                       &Window::resultItemTextFontSize, &Window::setResultItemTextFontSize);

        addFontSpinBox(bl, tr("Description font size"), &window,
                       &Window::resultItemSubtextFontSize, &Window::setResultItemSubtextFontSize);

        addPixelMetricSpinBox(bl, tr("Selection border radius"), &window,
                              &Window::resultItemSelectionBorderRadius, &Window::setResultItemSelectionBorderRadius);

        addPixelMetricSpinBox(bl, tr("Selection border width"), &window,
                              &Window::resultItemSelectionBorderWidth, &Window::setResultItemSelectionBorderWidth);

        addPixelMetricSpinBox(bl, tr("Padding"), &window,
                              &Window::resultItemPadding, &Window::setResultItemPadding);

        addPixelMetricSpinBox(bl, tr("Icon size"), &window,
                              &Window::resultItemIconSize, &Window::setResultItemIconSize);

        addPixelMetricSpinBox(bl, tr("Horizontal spacing"), &window,
                              &Window::resultItemHorizontalSpace, &Window::setResultItemHorizontalSpace);

        addPixelMetricSpinBox(bl, tr("Vertical spacing"), &window,
                              &Window::resultItemVerticalSpace, &Window::setResultItemVerticalSpace);

        wl->addWidget(b);
        b = new QGroupBox(tr("Actions"));
        bl = new QFormLayout(b);

        addFontSpinBox(bl, tr("Font size"), &window,
                       &Window::actionItemFontSize, &Window::setActionItemFontSize);

        addPixelMetricSpinBox(bl, tr("Selection border radius"), &window,
                              &Window::actionItemSelectionBorderRadius, &Window::setActionItemSelectionBorderRadius);

        addPixelMetricSpinBox(bl, tr("Selection border width"), &window,
                              &Window::actionItemSelectionBorderWidth, &Window::setActionItemSelectionBorderWidth);

        addPixelMetricSpinBox(bl, tr("Padding"), &window,
                              &Window::actionItemPadding, &Window::setActionItemPadding);

        wl->addWidget(b);
        w->setWindowTitle(tr("Window properties"));
        w->setWindowModality(Qt::WindowModality::WindowModal);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });


}
