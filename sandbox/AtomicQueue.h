#ifndef __ATOMIC_QUEUE_H__
#define __ATOMIC_QUEUE_H__

#include <atomic>
#include <vector>
#include <memory>

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
    ~AtomicQueueNode() {
        std::cout<<"YAY ";
        std::cout.flush();
        std::cout<<next.get()<<" WHEY ";
        if (next.get()!=nullptr) {
            std::cout.flush();
            std::cout<<next->v;
            std::cout.flush();
            std::cout<<" BAY"<<std::endl;
        }
        else {
            std::cout<<"RAY"<<std::endl;
        }
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
        auto node = std::make_shared<AtomicQueueNode<T>>(v);
        while(true) {
            auto last = std::atomic_load(&tail);
            auto next = last->next;
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
        return T();
    }
    bool empty() {
        return head.get()==tail.get();
    }
private:
    std::shared_ptr<AtomicQueueNode<T>> head, tail;
};

#endif
