#ifndef USE_CONV_LOCKFREEQUEUE_H
#define USE_CONV_LOCKFREEQUEUE_H

#include <atomic>
#include <vector>

namespace hzd
{
    template<typename T>
    class LockFreeQueue {
    private:
        struct Node {
            T data;
            std::atomic<Node*> next;

            Node(const T& data) : data(data), next(nullptr) {}
        };

        std::atomic<Node*> head;
        std::atomic<Node*> tail;

    public:
        LockFreeQueue() : head(new Node(T())), tail(head.load()) {}

        ~LockFreeQueue() {
            while(head != nullptr) {
                Node* temp = head;
                head.store(head.load()->next);
                delete temp;
            }
        }

        void push(const T& data) {
            Node* newNode = new Node(data);
            Node* prevTail = tail.exchange(newNode);
            prevTail->next = newNode;
        }

        bool pop(T& result) {
            Node* prevHead = head;
            Node* newHead;

            do {
                if(prevHead->next == nullptr) {
                    result = nullptr;
                    return false;
                }
                newHead = prevHead->next;
            } while(!std::atomic_compare_exchange_weak(&head, &prevHead, newHead));

            result = newHead->data;
            delete prevHead;
            return true;
        }
    };
}

#endif
