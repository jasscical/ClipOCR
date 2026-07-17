#include "HistoryManager.h"

HistoryManager::HistoryManager(QObject* parent) : QObject(parent) {}

void HistoryManager::addRecord(const QImage& image, const QString& str_text)
{
    HistoryItem item;
    item.m_img = image;
    item.m_strText = str_text;
    item.m_dtCreated = QDateTime::currentDateTime();
    m_listRecords.prepend(item);
    emit recordAdded(0);
}

int HistoryManager::count() const
{
    return m_listRecords.size();
}

const HistoryItem& HistoryManager::at(int i_index) const
{
    return m_listRecords.at(i_index);
}

void HistoryManager::clear()
{
    if (m_listRecords.isEmpty())
        return;
    m_listRecords.clear();
    emit cleared();
}

bool HistoryManager::isEmpty() const
{
    return m_listRecords.isEmpty();
}
