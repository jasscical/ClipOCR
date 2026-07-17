#include "MainWindow.h"
#include "Config.h"
#include "HistoryManager.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(Config* p_config, HistoryManager* p_history, QWidget* parent)
    : QMainWindow(parent), m_pConfig(p_config), m_pHistory(p_history)
{
    buildUi();

    connect(m_pList, &QListWidget::itemClicked, this, &MainWindow::onListItemClicked);
    connect(m_pCopyTextBtn, &QPushButton::clicked, this, &MainWindow::onCopyText);
    connect(m_pCopyImageBtn, &QPushButton::clicked, this, &MainWindow::onCopyImage);
    connect(m_pLangCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);
    connect(m_pSettingsAct, &QAction::triggered, this, &MainWindow::settingsRequested);
    connect(m_pClearAct, &QAction::triggered, this, &MainWindow::clearHistoryRequested);
    connect(m_pAboutAct, &QAction::triggered, this, &MainWindow::onAbout);
    connect(m_pHistory, &HistoryManager::recordAdded, this, &MainWindow::onHistoryRecordAdded);
    connect(m_pHistory, &HistoryManager::cleared, this, &MainWindow::onHistoryCleared);

    // Replay any records that already exist (e.g. loaded later).
    for (int i = m_pHistory->count() - 1; i >= 0; --i)
        addListItem(m_pHistory->at(i), i);
    refreshEmptyState();

    retranslateUi();
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("ClipOCR"));
    resize(900, 560);

    auto* p_central = new QWidget(this);
    setCentralWidget(p_central);
    auto* p_root = new QVBoxLayout(p_central);

    // Toolbar
    auto* p_tool = new QToolBar(this);
    p_tool->setMovable(false);
    m_pSettingsAct = p_tool->addAction(QStringLiteral("Settings"));
    m_pClearAct = p_tool->addAction(QStringLiteral("Clear History"));
    p_tool->addSeparator();
    m_pLangLabel = new QLabel(QStringLiteral("Language:"), this);
    p_tool->addWidget(m_pLangLabel);
    m_pLangCombo = new QComboBox(this);
    m_pLangCombo->addItem(QStringLiteral("English"), QStringLiteral("en"));
    m_pLangCombo->addItem(QStringLiteral("Chinese"), QStringLiteral("zh_CN"));
    const int i_langIdx = (m_pConfig && m_pConfig->language() == QStringLiteral("zh_CN")) ? 1 : 0;
    m_pLangCombo->setCurrentIndex(i_langIdx);
    p_tool->addWidget(m_pLangCombo);
    p_tool->addSeparator();
    m_pAboutAct = p_tool->addAction(QStringLiteral("About"));
    addToolBar(p_tool);

    // Splitter: list (left) | detail (right)
    auto* p_split = new QSplitter(Qt::Horizontal, this);
    p_split->setChildrenCollapsible(false);

    // Left: history list
    auto* p_left = new QWidget(this);
    auto* p_leftLayout = new QVBoxLayout(p_left);
    p_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_pHistoryLabel = new QLabel(QStringLiteral("History"), p_left);
    m_pHistoryLabel->setStyleSheet(QStringLiteral("font-weight:bold; padding:4px 8px;"));
    p_leftLayout->addWidget(m_pHistoryLabel);
    m_pList = new QListWidget(p_left);
    m_pList->setIconSize(QSize(56, 56));
    m_pList->setSpacing(4);
    p_leftLayout->addWidget(m_pList);

    // Right: detail
    auto* p_right = new QWidget(this);
    auto* p_rightLayout = new QVBoxLayout(p_right);
    p_rightLayout->setContentsMargins(8, 8, 8, 8);

    auto* p_scroll = new QScrollArea(p_right);
    p_scroll->setWidgetResizable(true);
    p_scroll->setMaximumHeight(320);
    m_pImageLabel = new QLabel(p_scroll);
    m_pImageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    p_scroll->setWidget(m_pImageLabel);
    p_rightLayout->addWidget(p_scroll);

    m_pTimeLabel = new QLabel(p_right);
    p_rightLayout->addWidget(m_pTimeLabel);

    m_pTextEdit = new QPlainTextEdit(p_right);
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setMinimumHeight(140);
    p_rightLayout->addWidget(m_pTextEdit);

    auto* p_btnRow = new QHBoxLayout();
    m_pCopyTextBtn = new QPushButton(QStringLiteral("Copy Text"), p_right);
    m_pCopyImageBtn = new QPushButton(QStringLiteral("Copy Image"), p_right);
    p_btnRow->addWidget(m_pCopyTextBtn);
    p_btnRow->addWidget(m_pCopyImageBtn);
    p_btnRow->addStretch();
    p_rightLayout->addLayout(p_btnRow);

    p_split->addWidget(p_left);
    p_split->addWidget(p_right);
    p_split->setStretchFactor(0, 0);
    p_split->setStretchFactor(1, 1);
    p_root->addWidget(p_split);
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("ClipOCR"));
    m_pSettingsAct->setText(tr("Settings"));
    m_pClearAct->setText(tr("Clear History"));
    m_pAboutAct->setText(tr("About"));
    m_pLangLabel->setText(tr("Language:"));
    m_pLangCombo->setItemText(0, tr("English"));
    m_pLangCombo->setItemText(1, tr("Chinese"));
    m_pHistoryLabel->setText(tr("History"));
    m_pCopyTextBtn->setText(tr("Copy Text"));
    m_pCopyImageBtn->setText(tr("Copy Image"));
    refreshEmptyState();
    if (m_iCurrentIndex >= 0)
        showDetail(m_iCurrentIndex);
}

void MainWindow::refreshEmptyState()
{
    if (m_pHistory->isEmpty() && m_iCurrentIndex < 0) {
        m_pTextEdit->setPlainText(tr(
            "No history yet. Capture an image to the clipboard and press the hotkey to begin."));
        m_pTimeLabel->clear();
        m_pImageLabel->clear();
        m_pCopyTextBtn->setEnabled(false);
        m_pCopyImageBtn->setEnabled(false);
    } else {
        m_pCopyTextBtn->setEnabled(m_iCurrentIndex >= 0);
        m_pCopyImageBtn->setEnabled(m_iCurrentIndex >= 0);
    }
}

void MainWindow::addListItem(const HistoryItem& item, int i_index)
{
    auto* p_widget = new QWidget();
    auto* p_layout = new QHBoxLayout(p_widget);
    p_layout->setContentsMargins(6, 4, 6, 4);
    p_layout->setSpacing(8);

    auto* p_thumb = new QLabel(p_widget);
    const QPixmap pix = QPixmap::fromImage(item.m_img).scaled(
        56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p_thumb->setPixmap(pix);
    p_thumb->setFixedSize(56, 56);

    auto* p_meta = new QVBoxLayout();
    p_meta->setContentsMargins(0, 0, 0, 0);
    p_meta->setSpacing(2);

    QString str_preview = item.m_strText;
    str_preview.replace(QLatin1Char('\n'), QLatin1Char(' '));
    if (str_preview.size() > 48)
        str_preview = str_preview.left(48) + QStringLiteral("...");
    auto* p_text = new QLabel(str_preview, p_widget);
    p_text->setWordWrap(true);
    auto* p_time = new QLabel(item.m_dtCreated.toString(QStringLiteral("yyyy-MM-dd HH:mm")), p_widget);
    p_time->setStyleSheet(QStringLiteral("color:#888;"));

    p_meta->addWidget(p_text);
    p_meta->addWidget(p_time);
    p_layout->addWidget(p_thumb);
    p_layout->addLayout(p_meta, 1);

    auto* p_item = new QListWidgetItem(m_pList);
    p_item->setData(Qt::UserRole, i_index);
    m_pList->insertItem(0, p_item);
    m_pList->setItemWidget(p_item, p_widget);
}

void MainWindow::showDetail(int i_index)
{
    if (i_index < 0 || i_index >= m_pHistory->count())
        return;
    m_iCurrentIndex = i_index;
    const HistoryItem& item = m_pHistory->at(i_index);
    m_dtCurrent = item.m_dtCreated;

    const QPixmap pix = QPixmap::fromImage(item.m_img).scaled(
        460, 460, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pImageLabel->setPixmap(pix);
    m_pTextEdit->setPlainText(item.m_strText);
    m_pTimeLabel->setText(tr("Time:") + QStringLiteral(" ") + item.m_dtCreated.toString());
    m_pCopyTextBtn->setEnabled(true);
    m_pCopyImageBtn->setEnabled(true);
}

void MainWindow::onListItemClicked(QListWidgetItem* p_item)
{
    if (!p_item)
        return;
    const int i_index = p_item->data(Qt::UserRole).toInt();
    showDetail(i_index);
}

void MainWindow::onCopyText()
{
    if (m_iCurrentIndex < 0 || m_iCurrentIndex >= m_pHistory->count())
        return;
    const HistoryItem& item = m_pHistory->at(m_iCurrentIndex);
    QApplication::clipboard()->setText(item.m_strText);
    m_pTimeLabel->setText(tr("Time:") + QStringLiteral(" ") + m_dtCurrent.toString()
                          + QStringLiteral("  (") + tr("Copied to clipboard") + QStringLiteral(")"));
}

void MainWindow::onCopyImage()
{
    if (m_iCurrentIndex < 0 || m_iCurrentIndex >= m_pHistory->count())
        return;
    const HistoryItem& item = m_pHistory->at(m_iCurrentIndex);
    QApplication::clipboard()->setImage(item.m_img);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("ClipOCR"),
        tr("ClipOCR - clipboard image to text.\n\n"
           "Capture an image to the clipboard (e.g. Snipaste), then press the hotkey "
           "to OCR it and copy the text."));
}

void MainWindow::onLanguageChanged(int i_index)
{
    const QString str_lang = m_pLangCombo->itemData(i_index).toString();
    emit languageChanged(str_lang);
}

void MainWindow::onHistoryRecordAdded(int i_index)
{
    addListItem(m_pHistory->at(i_index), i_index);
    refreshEmptyState();
}

void MainWindow::onHistoryCleared()
{
    m_pList->clear();
    m_iCurrentIndex = -1;
    refreshEmptyState();
}

void MainWindow::closeEvent(QCloseEvent* p_event)
{
    // Keep running in the tray; just hide the window.
    hide();
    p_event->accept();
}

void MainWindow::changeEvent(QEvent* p_event)
{
    if (p_event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(p_event);
}
