#ifndef MODULE_TCP_SENDQUEUE_H
#define MODULE_TCP_SENDQUEUE_H

#include <QByteArray>
#include <QMutex>
#include <QQueue>

/**
 * @file SendQueue.h
 * @brief 线程安全发送队列（P0 Reject；P1 DropOldest / peek）
 */
class SendQueue {
public:
    explicit SendQueue(int maxSize = 100);

    void setMaxSize(int maxSize);
    int maxSize() const;
    int size() const;
    bool isEmpty() const;

    /** @return false 表示队列已满，未入队 */
    bool tryEnqueue(const QByteArray& frame);

    /** 满则丢最旧再入队 */
    bool tryEnqueueDropOldest(const QByteArray& frame);

    /** 窥视队首，不出队 */
    bool tryPeek(QByteArray& out) const;

    /** @return false 表示空 */
    bool tryDequeue(QByteArray& out);

    void clear();

private:
    mutable QMutex m_mutex;
    QQueue<QByteArray> m_queue;
    int m_maxSize = 100;
};

#endif // MODULE_TCP_SENDQUEUE_H
