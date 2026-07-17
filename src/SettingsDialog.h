#pragma once

#include <QDialog>

class Config;
class QSpinBox;
class QCheckBox;
class QKeySequenceEdit;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

// ClipOCR - clipboard OCR utility.
// Settings dialog: hotkey, upscale, popup, Tesseract path, tessdata dir, language.
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(Config* p_config, QWidget* parent = nullptr);

    // Reload all fields from the current Config (call before show()).
    void loadFromConfig();

signals:
    void settingsApplied();

private slots:
    void onSave();
    void onReset();
    void onBrowseTessdata();

private:
    void retranslateUi();

    Config* m_pConfig = nullptr;
    QLabel* m_pHotkeyLbl = nullptr;
    QLabel* m_pUpscaleLbl = nullptr;
    QLabel* m_pPopupLbl = nullptr;
    QLabel* m_pTessPathLbl = nullptr;
    QLabel* m_pTessdataLbl = nullptr;
    QLabel* m_pLangLbl = nullptr;
    QKeySequenceEdit* m_pHotkeyEdit = nullptr;
    QSpinBox* m_pUpscaleSpin = nullptr;
    QCheckBox* m_pPopupCheck = nullptr;
    QLineEdit* m_pTessPathEdit = nullptr;
    QLineEdit* m_pTessdataEdit = nullptr;
    QPushButton* m_pBrowseBtn = nullptr;
    QComboBox* m_pLangCombo = nullptr;
    QPushButton* m_pSaveBtn = nullptr;
    QPushButton* m_pResetBtn = nullptr;
    QLabel* m_pHintLabel = nullptr;
};
