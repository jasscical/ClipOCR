#include "SettingsDialog.h"
#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

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

    m_pLangCombo = new QComboBox(this);
    m_pLangCombo->addItem(QStringLiteral("English"), QStringLiteral("eng"));
    m_pLangCombo->addItem(QStringLiteral("Chinese (Simp.)"), QStringLiteral("chi_sim+eng"));
    m_pLangCombo->setCurrentIndex(
        (m_pConfig->language().contains(QStringLiteral("chi_sim"))) ? 1 : 0);

    m_pHotkeyLbl = new QLabel(this);
    m_pUpscaleLbl = new QLabel(this);
    m_pPopupLbl = new QLabel(this);
    m_pTessPathLbl = new QLabel(this);
    m_pTessdataLbl = new QLabel(this);
    m_pLangLbl = new QLabel(this);

    p_form->addRow(m_pHotkeyLbl, m_pHotkeyEdit);
    p_form->addRow(m_pUpscaleLbl, m_pUpscaleSpin);
    p_form->addRow(m_pPopupLbl, m_pPopupCheck);
    p_form->addRow(m_pTessPathLbl, m_pTessPathEdit);
    p_form->addRow(m_pTessdataLbl, p_pathRow);
    p_form->addRow(m_pLangLbl, m_pLangCombo);
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

void SettingsDialog::loadFromConfig()
{
    m_pHotkeyEdit->setKeySequence(m_pConfig->hotkey());
    m_pUpscaleSpin->setValue(m_pConfig->upscaleFactor());
    m_pPopupCheck->setChecked(m_pConfig->showPopup());
    m_pTessPathEdit->setText(m_pConfig->tesseractPath());
    m_pTessdataEdit->setText(m_pConfig->tessdataDir());
    m_pLangCombo->setCurrentIndex(
        (m_pConfig->language().contains(QStringLiteral("chi_sim"))) ? 1 : 0);
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    m_pHotkeyLbl->setText(tr("Hotkey"));
    m_pUpscaleLbl->setText(tr("Upscale factor (1-4)"));
    m_pPopupLbl->setText(tr("Show popup on success"));
    m_pTessPathLbl->setText(tr("Tesseract path"));
    m_pTessdataLbl->setText(tr("Tessdata directory"));
    m_pBrowseBtn->setText(tr("Browse..."));
    m_pLangLbl->setText(tr("Language(s)"));
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
    m_pConfig->setTesseractPath(m_pTessPathEdit->text().trimmed());
    m_pConfig->setTessdataDir(m_pTessdataEdit->text().trimmed());
    m_pConfig->setLanguage(m_pLangCombo->itemData(m_pLangCombo->currentIndex()).toString());
    m_pConfig->save();
    emit settingsApplied();
    accept();
}

void SettingsDialog::onReset()
{
    m_pConfig->setHotkey(QKeySequence(QStringLiteral("Ctrl+Alt+O")));
    m_pConfig->setUpscaleFactor(3);
    m_pConfig->setShowPopup(true);
    m_pConfig->setTesseractPath(QStringLiteral("tesseract"));
    m_pConfig->setTessdataDir(QString());
    m_pConfig->setLanguage(Config::defaultLanguage());
    m_pConfig->save();
    loadFromConfig();
    emit settingsApplied();
}
