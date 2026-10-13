#include "SendQueue.h"

SendQueue::SendQueue(int maxSize)
    : m_maxSize(maxSize > 0 ? maxSize : 1)
{
}

void SendQueue::setMaxSize(int maxSize)
{
    QMutexLocker lock(&m_mutex);
    m_maxSize = maxSize > 0 ? maxSize : 1;
}

int SendQueue::maxSize() const
{
    QMutexLocker lock(&m_mutex);
    return m_maxSize;
}

int SendQueue::size() const
{
    QMutexLocker lock(&m_mutex);
    return m_queue.size();
}

bool SendQueue::isEmpty() const
{
    QMutexLocker lock(&m_mutex);
    return m_queue.isEmpty();
}

bool SendQueue::tryEnqueue(const QByteArray& frame)
{
    QMutexLocker lock(&m_mutex);
    if (m_queue.size() >= m_maxSize)
        return false;
    m_queue.enqueue(frame);
    return true;
}

bool SendQueue::tryEnqueueDropOldest(const QByteArray& frame)
{
    QMutexLocker lock(&m_mutex);
    while (m_queue.size() >= m_maxSize && !m_queue.isEmpty())
        m_queue.dequeue();
    m_queue.enqueue(frame);
    return true;
}

bool SendQueue::tryPeek(QByteArray& out) const
{
    QMutexLocker lock(&m_mutex);
    if (m_queue.isEmpty())
        return false;
    out = m_queue.head();
    return true;
}

bool SendQueue::tryDequeue(QByteArray& out)
{
    QMutexLocker lock(&m_mutex);
    if (m_queue.isEmpty())
        return false;
    out = m_queue.dequeue();
    return true;
}

void SendQueue::clear()
{
    QMutexLocker lock(&m_mutex);
    m_queue.clear();
}
