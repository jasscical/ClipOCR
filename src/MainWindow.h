#pragma once

#include <QMainWindow>
#include <QDateTime>

class Config;
class HistoryManager;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QAction;
class QComboBox;

// ClipOCR - clipboard OCR utility.
// Main window: shows OCR history (image + recognized text) and provides
// access to Settings and the UI language switch.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Config* p_config, HistoryManager* p_history, QWidget* parent = nullptr);

signals:
    void settingsRequested();
    void languageChanged(const QString& str_lang);
    void clearHistoryRequested();

protected:
    void closeEvent(QCloseEvent* p_event) override;
    void changeEvent(QEvent* p_event) override;

private slots:
    void onListItemClicked(QListWidgetItem* p_item);
    void onCopyText();
    void onCopyImage();
    void onAbout();
    void onLanguageChanged(int i_index);
    void onHistoryRecordAdded(int i_index);
    void onHistoryCleared();

private:
    void buildUi();
    void retranslateUi();
    void showDetail(int i_index);
    void addListItem(const class HistoryItem& item, int i_index);
    void refreshEmptyState();

    Config* m_pConfig = nullptr;
    HistoryManager* m_pHistory = nullptr;

    QListWidget* m_pList = nullptr;
    QLabel* m_pImageLabel = nullptr;
    QPlainTextEdit* m_pTextEdit = nullptr;
    QLabel* m_pTimeLabel = nullptr;
    QPushButton* m_pCopyTextBtn = nullptr;
    QPushButton* m_pCopyImageBtn = nullptr;

    QLabel* m_pHistoryLabel = nullptr;
    QLabel* m_pLangLabel = nullptr;
    QComboBox* m_pLangCombo = nullptr;
    QAction* m_pSettingsAct = nullptr;
    QAction* m_pClearAct = nullptr;
    QAction* m_pAboutAct = nullptr;

    int m_iCurrentIndex = -1;
    QDateTime m_dtCurrent;
};
