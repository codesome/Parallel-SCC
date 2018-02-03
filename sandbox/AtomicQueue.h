#ifndef __ATOMIC_QUEUE_H__
#define __ATOMIC_QUEUE_H__

#include <atomic>
#include <vector>
#include <memory>
#include <sstream>

template <typename T>
struct AtomicQueueNode {
    T v;
    std::shared_ptr<AtomicQueueNode<T>> next;
    AtomicQueueNode(T v=T()): v(v) {
        next = std::shared_ptr<AtomicQueueNode<T>>(nullptr);
    }
};

// template <typename T>
// class AtomicEnDqQueue {
// public:
//     AtomicEnDqQueue() {
//         head = std::make_shared<AtomicQueueNode<T>>();
//         tail = head;
//     }
//     void enqueue(T v) {
//         std::shared_ptr<AtomicQueueNode<T>> newNode = std::make_shared<AtomicQueueNode<T>>(v);
//         while(true) {
//             std::shared_ptr<AtomicQueueNode<T>> last = std::atomic_load(&tail);
//             std::shared_ptr<AtomicQueueNode<T>> next = std::atomic_load(&(last->next));
//             if (tail.get()==last.get()) {
//                 if(next.get()==nullptr) {
//                     if(atomic_compare_exchange_strong(&(last->next), &next, newNode)) {
//                         atomic_compare_exchange_strong(&tail, &last, newNode);
//                         return;
//                     }
//                 } else {
//                     atomic_compare_exchange_strong(&tail, &last, next);
//                 }
//             }
//         }
//     }
//     T dequeue() {
//         while(true) {
//             std::shared_ptr<AtomicQueueNode<T>> first = head;
//             std::shared_ptr<AtomicQueueNode<T>> last = tail;
//             std::shared_ptr<AtomicQueueNode<T>> next = first->next;
//             if(first.get()==last.get()) {
//                 if(next.get()!=nullptr)
//                     atomic_compare_exchange_strong(&tail, &last, next);
//             } else {
//                 T val = next->v;
//                 if(atomic_compare_exchange_strong(&head, &first, next)) {
//                     return val;
//                 }
//             }
//         }
//     }
//     bool empty() {
//         return head.get()==tail.get();
//     }
// private:
//     std::shared_ptr<AtomicQueueNode<T>> head, tail;
// };

template <typename T>
class AtomicEnDqQueue {
public:
    AtomicEnDqQueue() {
        head = std::make_shared<AtomicQueueNode<T>>();
        tail = head;
    }
    void enqueue(T v) {
        std::shared_ptr<AtomicQueueNode<T>> node = std::make_shared<AtomicQueueNode<T>>(v);
        while(true) {
            std::shared_ptr<AtomicQueueNode<T>> last = std::atomic_load(&tail);
            std::shared_ptr<AtomicQueueNode<T>> next = last->next;
            decltype(next) zero;
            if (next.get()==nullptr) {
                if (std::atomic_compare_exchange_strong(&(last->next),&(next),node)) {
                    std::atomic_compare_exchange_strong(&(tail),&(last),node);
                    return;
                }
            }
            else {
                std::atomic_compare_exchange_strong(&(tail),&(last),next);
            }
        }
    }
    T dequeue() {
        while(true) {
            std::shared_ptr<AtomicQueueNode<T>> first = std::atomic_load(&head);
            std::shared_ptr<AtomicQueueNode<T>> last = std::atomic_load(&tail);
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
    ~AtomicEnDqQueue() {
        auto ptr=head;
        auto next=ptr->next;
        head=nullptr;
        tail=nullptr;
        while(next.get()!=nullptr) {
            ptr=next;
            next=ptr->next;
        }
    }
private:
    std::shared_ptr<AtomicQueueNode<T>> head, tail;
};

#endif
