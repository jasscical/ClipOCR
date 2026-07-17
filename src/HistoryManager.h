#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QDateTime>
#include <QList>

// ClipOCR - clipboard OCR utility.
// A single OCR result: the captured image, the recognized text, and a timestamp.
struct HistoryItem
{
    QImage m_img;          // captured (clipboard) image
    QString m_strText;     // recognized text
    QDateTime m_dtCreated; // when the record was created
};

// In-memory store of OCR history. Newest record is at index 0.
// (Single responsibility: manage the history list and notify listeners.)
class HistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit HistoryManager(QObject* parent = nullptr);

    // Prepend a new record (newest first). Emits recordAdded().
    void addRecord(const QImage& image, const QString& str_text);

    int count() const;
    const HistoryItem& at(int i_index) const;

    void clear();
    bool isEmpty() const;

signals:
    void recordAdded(int i_index); // index of the newly added record (always 0)
    void cleared();

private:
    QList<HistoryItem> m_listRecords;
};
