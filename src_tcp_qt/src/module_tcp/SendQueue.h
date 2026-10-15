#ifndef MODULE_TCP_SENDQUEUE_H
#define MODULE_TCP_SENDQUEUE_H

#include <QByteArray>
#include <QMutex>
#include <QQueue>

/**
 * @file SendQueue.h
 * @brief 线程安全的发送队列（门面与 worker 均可访问）。
 */
class SendQueue {
public:
    /**
     * @brief 构造队列。
     * @param maxSize 最大帧数；无效值按 1 处理
     */
    explicit SendQueue(int maxSize = 100);

    /**
     * @brief 设置队列上限。
     * @param maxSize 最大帧数
     */
    void setMaxSize(int maxSize);

    /**
     * @brief 当前上限。
     */
    int maxSize() const;

    /**
     * @brief 当前帧数。
     */
    int size() const;

    /**
     * @brief 是否为空。
     * @return true 空
     */
    bool isEmpty() const;

    /**
     * @brief 尝试入队（满则失败）。
     * @param frame 待发帧
     * @return true 已入队；false 已满未入队
     */
    bool tryEnqueue(const QByteArray& frame);

    /**
     * @brief 满则丢最旧再入队。
     * @param frame 待发帧
     * @return 始终 true（入队成功）
     */
    bool tryEnqueueDropOldest(const QByteArray& frame);

    /**
     * @brief 窥视队首，不出队。
     * @param[out] out 队首副本
     * @return true 非空；false 空
     */
    bool tryPeek(QByteArray& out) const;

    /**
     * @brief 出队队首。
     * @param[out] out 出队帧
     * @return true 成功；false 空
     */
    bool tryDequeue(QByteArray& out);

    /**
     * @brief 清空队列。
     */
    void clear();

private:
    mutable QMutex m_mutex;
    QQueue<QByteArray> m_queue;
    int m_maxSize = 100;
};

#endif // MODULE_TCP_SENDQUEUE_H
