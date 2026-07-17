#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QDateTime>
#include <QList>

// ClipOCR - 剪贴板 OCR 小工具。
// 单条 OCR 记录：抓取的图片、识别出的文字、以及时间戳。
struct HistoryItem
{
    QImage m_img;          // 抓取的（剪贴板）图片
    QString m_strText;     // 识别出的文字
    QDateTime m_dtCreated; // 记录创建时间
};

// OCR 历史的内存存储。最新记录位于索引 0。
// （单一职责：管理历史列表并通知监听者。）
class HistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit HistoryManager(QObject* parent = nullptr);

    // 在头部插入新记录（最新在前）。会发出 recordAdded() 信号。
    void addRecord(const QImage& image, const QString& str_text);

    int count() const;
    const HistoryItem& at(int i_index) const;

    void clear();
    bool isEmpty() const;

signals:
    void recordAdded(int i_index); // 新添加记录的索引（始终为 0）
    void cleared();

private:
    QList<HistoryItem> m_listRecords;
};
