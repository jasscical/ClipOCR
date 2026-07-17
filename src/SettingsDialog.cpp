#include "SettingsDialog.h"
#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// 友好名称（可翻译）映射到 Tesseract 语言代码。
// 代码作为 itemData 保存并写入 Config；名称仅用于显示。
static const QVector<QPair<QString, const char*>>& ocrLangOptions()
{
    static const QVector<QPair<QString, const char*>> s_list = {
        {QStringLiteral("chi_sim+eng"), "Chinese + English"},
        {QStringLiteral("eng"),          "English"},
        {QStringLiteral("chi_sim"),      "Chinese (simplified)"},
        {QStringLiteral("chi_tra"),      "Chinese (traditional)"},
        {QStringLiteral("jpn"),          "Japanese"},
        {QStringLiteral("kor"),          "Korean"},
        {QStringLiteral("fra"),          "French"},
        {QStringLiteral("deu"),          "German"},
        {QStringLiteral("spa"),          "Spanish"},
        {QStringLiteral("rus"),          "Russian"},
    };
    return s_list;
}

SettingsDialog::SettingsDialog(Config* p_config, QWidget* parent)
    : QDialog(parent), m_pConfig(p_config)
{
    setWindowTitle(QStringLiteral("Settings"));
    setMinimumWidth(460);

    auto* p_root = new QVBoxLayout(this);
    auto* p_form = new QFormLayout();

    m_pHotkeyEdit = new QKeySequenceEdit(m_pConfig->hotkey(), this);
    m_pUpscaleSpin = new QSpinBox(this);
    m_pUpscaleSpin->setRange(1, 4);
    m_pPopupCheck = new QCheckBox(this);

    m_pTessPathEdit = new QLineEdit(this);

    m_pTessdataEdit = new QLineEdit(this);
    m_pBrowseBtn = new QPushButton(QStringLiteral("Browse..."), this);
    auto* p_pathRow = new QHBoxLayout();
    p_pathRow->addWidget(m_pTessdataEdit, 1);
    p_pathRow->addWidget(m_pBrowseBtn);

    m_pOcrLangCombo = new QComboBox(this);

    m_pHotkeyLbl = new QLabel(this);
    m_pUpscaleLbl = new QLabel(this);
    m_pPopupLbl = new QLabel(this);
    m_pTessPathLbl = new QLabel(this);
    m_pTessdataLbl = new QLabel(this);
    m_pOcrLangLbl = new QLabel(this);

    p_form->addRow(m_pHotkeyLbl, m_pHotkeyEdit);
    p_form->addRow(m_pUpscaleLbl, m_pUpscaleSpin);
    p_form->addRow(m_pPopupLbl, m_pPopupCheck);
    p_form->addRow(m_pOcrLangLbl, m_pOcrLangCombo);
    p_form->addRow(m_pTessPathLbl, m_pTessPathEdit);
    p_form->addRow(m_pTessdataLbl, p_pathRow);
    p_root->addLayout(p_form);

    m_pHintLabel = new QLabel(this);
    m_pHintLabel->setWordWrap(true);
    p_root->addWidget(m_pHintLabel);

    auto* p_btnRow = new QHBoxLayout();
    m_pSaveBtn = new QPushButton(QStringLiteral("Save"), this);
    m_pResetBtn = new QPushButton(QStringLiteral("Reset to defaults"), this);
    p_btnRow->addWidget(m_pSaveBtn);
    p_btnRow->addWidget(m_pResetBtn);
    p_btnRow->addStretch();
    p_root->addLayout(p_btnRow);

    connect(m_pSaveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(m_pResetBtn, &QPushButton::clicked, this, &SettingsDialog::onReset);
    connect(m_pBrowseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseTessdata);

    loadFromConfig();
    retranslateUi();
}

void SettingsDialog::changeEvent(QEvent* p_event)
{
    if (p_event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(p_event);
}

void SettingsDialog::loadFromConfig()
{
    m_pHotkeyEdit->setKeySequence(m_pConfig->hotkey());
    m_pUpscaleSpin->setValue(m_pConfig->upscaleFactor());
    m_pPopupCheck->setChecked(m_pConfig->showPopup());
    m_pTessPathEdit->setText(m_pConfig->tesseractPath());
    m_pTessdataEdit->setText(m_pConfig->tessdataDir());
    const int iIdx = m_pOcrLangCombo->findData(m_pConfig->ocrLanguage());
    m_pOcrLangCombo->setCurrentIndex(iIdx >= 0 ? iIdx : 0);
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    m_pHotkeyLbl->setText(tr("Hotkey"));
    m_pUpscaleLbl->setText(tr("Upscale factor (1-4)"));
    m_pPopupLbl->setText(tr("Show popup on success"));
    m_pOcrLangLbl->setText(tr("OCR recognition language"));
    m_pOcrLangCombo->setToolTip(tr(
        "Language of the text in the screenshots (Tesseract code, e.g. chi_sim+eng)."));
    // 用翻译后的名称重建下拉项，然后重新选中当前 OCR 语言。
    // 这样在切换界面语言（通过 changeEvent 触发本函数）时，选中项不会丢失。
    m_pOcrLangCombo->clear();
    for (const auto& opt : ocrLangOptions())
        m_pOcrLangCombo->addItem(tr(opt.second), opt.first);
    const int iIdx = m_pOcrLangCombo->findData(m_pConfig->ocrLanguage());
    m_pOcrLangCombo->setCurrentIndex(iIdx >= 0 ? iIdx : 0);
    m_pTessPathLbl->setText(tr("Tesseract path"));
    m_pTessdataLbl->setText(tr("Tessdata directory"));
    m_pBrowseBtn->setText(tr("Browse..."));
    m_pSaveBtn->setText(tr("Save"));
    m_pResetBtn->setText(tr("Reset to defaults"));
    m_pHintLabel->setText(tr(
        "Usage: screenshot to clipboard (e.g. Snipaste), then press the hotkey.\n"
        "Requires Tesseract OCR engine installed. Download chi_sim.traineddata for\n"
        "Chinese and place it in the tessdata directory."));
}

void SettingsDialog::onBrowseTessdata()
{
    const QString str_dir = QFileDialog::getExistingDirectory(
        this, tr("Select tessdata directory"), m_pTessdataEdit->text());
    if (!str_dir.isEmpty())
        m_pTessdataEdit->setText(str_dir);
}

void SettingsDialog::onSave()
{
    m_pConfig->setHotkey(m_pHotkeyEdit->keySequence());
    m_pConfig->setUpscaleFactor(m_pUpscaleSpin->value());
    m_pConfig->setShowPopup(m_pPopupCheck->isChecked());
    m_pConfig->setOcrLanguage(m_pOcrLangCombo->currentData().toString());
    m_pConfig->setTesseractPath(m_pTessPathEdit->text().trimmed());
    m_pConfig->setTessdataDir(m_pTessdataEdit->text().trimmed());
    m_pConfig->save();
    emit settingsApplied();
    accept();
}

void SettingsDialog::onReset()
{
    m_pConfig->setHotkey(QKeySequence(QStringLiteral("Ctrl+Alt+O")));
    m_pConfig->setUpscaleFactor(3);
    m_pConfig->setShowPopup(true);
    m_pConfig->setOcrLanguage(Config::defaultOcrLanguage());
    m_pConfig->setTesseractPath(QStringLiteral("tesseract"));
    m_pConfig->setTessdataDir(QString());
    m_pConfig->save();
    loadFromConfig();
    emit settingsApplied();
}
