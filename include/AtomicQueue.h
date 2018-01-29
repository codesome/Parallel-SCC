#ifndef __ATOMIC_QUEUE_H__
#define __ATOMIC_QUEUE_H__

#include <atomic>
#include <vector>
#include <memory>

template <typename T>
struct QueueNode {
    T v;
    QueueNode<T> *next;
    QueueNode(T v): v(v), next(nullptr) {}
    QueueNode(): next(nullptr) {}
};

template <typename T>
class AtomicEnQueue {
public:
    AtomicEnQueue() {
        head = new QueueNode<T>();
        tail.store(head);
        archive_head = new QueueNode<T>();
        archive_tail = archive_head;
    }
    void enqueue(T v) {
        QueueNode<T> *node = new QueueNode<T>(v);
        QueueNode<T> *pred = tail.exchange(node);
        pred->next = node;
    }
    QueueNode<T>* getFirstNode() {
        return head->next;
    }
    QueueNode<T>* getFlushedFirstNode() {
        return archive_head->next;
    }
    void clear() {
        flush();
        QueueNode<T> *qn = head->next;
        QueueNode<T> *next;
        while(qn!=nullptr) {
            next = qn->next;
            delete qn;
            qn = next;
        }
        head->next = nullptr;
    }
    void flush() {
        archive_tail->next = head->next;
        archive_tail = tail.load();
        head->next = nullptr;
        tail.store(head);
    }
    ~AtomicEnQueue() {
        clear();
        delete head;
        delete archive_head;
    }
private:
    std::atomic<QueueNode<T>*> tail;
    QueueNode<T> *head;
    QueueNode<T> *archive_head;
    QueueNode<T> *archive_tail;
};

/////////////////////

/*
template <typename T>
struct AtomicQueueNode {
    T v;
    std::shared_ptr<AtomicQueueNode<T>> next;
    AtomicQueueNode(T v): v(v) {
        next = std::shared_ptr<AtomicQueueNode<T>>(nullptr);
    }
    AtomicQueueNode() {
        next = std::shared_ptr<AtomicQueueNode<T>>(nullptr);
    }
};

template <typename T>
class AtomicEnDqQueue {
public:
    AtomicEnDqQueue() {
        head = std::make_shared<AtomicQueueNode<T>>();
        tail = head;
    }
    void enqueue(T v) {
        std::shared_ptr<AtomicQueueNode<T>> newNode = std::make_shared<AtomicQueueNode<T>>(v);
        while(true) {
            std::shared_ptr<AtomicQueueNode<T>> last = tail;
            std::shared_ptr<AtomicQueueNode<T>> next = last->next;
            if(next.get()==nullptr) {
                if(atomic_compare_exchange_strong(&(last->next), &next, newNode)) {
                    atomic_compare_exchange_strong(&tail, &last, newNode);
                    return;
                } 
            } else {
                atomic_compare_exchange_strong(&tail, &last, next);
            }
        }
    }
    T dequeue() {
        while(true) {
            std::shared_ptr<AtomicQueueNode<T>> first = head;
            std::shared_ptr<AtomicQueueNode<T>> last = tail;
            std::shared_ptr<AtomicQueueNode<T>> next = first->next;
            if(first.get()==last.get()) {
                if(next.get()!=nullptr)
                    atomic_compare_exchange_strong(&tail, &last, next);
            } else {
                T val = next->v;
                if(atomic_compare_exchange_strong(&head, &first, next)) {
                    return val;
                }
            }
        }
    }
    bool empty() {
        return head.get()==tail.get();
    }
private:
    std::shared_ptr<AtomicQueueNode<T>> head, tail;
};
*/

template <typename T>
struct AtomicQueueNode {
    T v;
    std::atomic<AtomicQueueNode<T>*> next;
    AtomicQueueNode(T v): v(v) {
        next.store(nullptr);
    }
    AtomicQueueNode() {
        next.store(nullptr);
    }
};

template <typename T>
class AtomicEnDqQueue {
public:
    AtomicEnDqQueue() {
        head.store(new AtomicQueueNode<T>());
        tail.store(head.load());
    }
    void enqueue(T v) {
        AtomicQueueNode<T> *newNode = new AtomicQueueNode<T>(v);
        while(true) {
            AtomicQueueNode<T> *last = tail.load();
            AtomicQueueNode<T> *next = (last->next).load();
            if(next==nullptr) {
                if((last->next).compare_exchange_strong(next, newNode)) {
                    tail.compare_exchange_strong(last, newNode);
                    return;
                } 
            } else {
                tail.compare_exchange_strong(last, next);
            }
        }
    }
    T dequeue() {
        while(true) {
            AtomicQueueNode<T> *first = head.load();
            AtomicQueueNode<T> *last = tail.load();
            AtomicQueueNode<T> *next = (first->next).load();
            if(first==last) {
                if(next!=nullptr)
                    tail.compare_exchange_strong(last, next);
            } else {
                T val = next->v;
                if(head.compare_exchange_strong(first, next)) {
                    return val;
                }
            }
        }
    }
    T weak_dequeue(bool &emp) {
        while(true) {
            if(empty()) {
                emp = true;
                return head.load()->v;
            }
            AtomicQueueNode<T> *first = head.load();
            AtomicQueueNode<T> *last = tail.load();
            AtomicQueueNode<T> *next = (first->next).load();
            if(first==last) {
                if(next!=nullptr)
                    tail.compare_exchange_strong(last, next);
            } else {
                T val = next->v;
                if(head.compare_exchange_strong(first, next)) {
                    emp = false;
                    return val;
                }
            }
        }
    }
    bool empty() {
        return head.load()==tail.load();
    }
    AtomicQueueNode<T>* getFirstNode() {
        return (head.load()->next).load();
    }
    void clear() {
        AtomicQueueNode<T> *qn = (head.load()->next).load();
        AtomicQueueNode<T> *next;
        while(qn!=nullptr) {
            next = (qn->next).load();
            delete qn;
            qn = next;
        }
        (head.load()->next).store(nullptr);
    }
    ~AtomicEnDqQueue() {
        clear();
        delete head.load();
    }
private:
    std::atomic<AtomicQueueNode<T>*> head, tail;
};

#endif