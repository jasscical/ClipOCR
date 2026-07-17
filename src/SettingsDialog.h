#pragma once

#include <QDialog>

class Config;
class QSpinBox;
class QCheckBox;
class QKeySequenceEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;

// ClipOCR - 剪贴板 OCR 小工具。
// 设置对话框：快捷键、放大倍数、弹窗、Tesseract 路径、tessdata 目录、OCR 语言。
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(Config* p_config, QWidget* parent = nullptr);

    // 从当前 Config 重新载入所有字段（show() 之前调用）。
    void loadFromConfig();

signals:
    void settingsApplied();

protected:
    void changeEvent(QEvent* p_event) override;

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
    QLabel* m_pOcrLangLbl = nullptr;
    QComboBox* m_pOcrLangCombo = nullptr;
    QKeySequenceEdit* m_pHotkeyEdit = nullptr;
    QSpinBox* m_pUpscaleSpin = nullptr;
    QCheckBox* m_pPopupCheck = nullptr;
    QLineEdit* m_pTessPathEdit = nullptr;
    QLineEdit* m_pTessdataEdit = nullptr;
    QPushButton* m_pBrowseBtn = nullptr;
    QPushButton* m_pSaveBtn = nullptr;
    QPushButton* m_pResetBtn = nullptr;
    QLabel* m_pHintLabel = nullptr;
};
