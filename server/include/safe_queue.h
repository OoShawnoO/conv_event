#ifndef USE_CONV_SAFE_QUEUE_H
#define USE_CONV_SAFE_QUEUE_H

#include <atomic>
#include <vector>

namespace hzd
{
    template<typename T>
    class safe_queue {
    private:
        struct Node {
            T data;
            Node* next;
            Node() = default;
            explicit Node(const T& data) : data(data), next(nullptr) {}
        };

        std::atomic<Node*> head;
        std::atomic<Node*> tail;
        std::atomic<Node*> node_pool;
        std::atomic<int> pool_size;

        Node* acquire_node() {
            Node* node = node_pool.load(std::memory_order_acquire);
            while (node) {
                Node* next = node->next;
                if (node_pool.compare_exchange_strong(node, next)) {
                    return node;
                }
            }
            return new Node();
        }
        void release_node(Node* node) {
            Node* old_pool = node_pool.load();
            node->next = old_pool;
            while (!node_pool.compare_exchange_strong(old_pool, node)) {
                node->next = old_pool;
            }
        }
    public:
        explicit safe_queue(int pool_size = 1024)
        : head(new Node(T())), tail(head.load()), node_pool(new Node[pool_size]), pool_size(pool_size)
        {
            for(int i = 0 ; i < pool_size - 1 ; i++)
            {
                node_pool[i].next = &node_pool[i+1];
            }
            node_pool[pool_size-1].next = nullptr;
        }

        ~safe_queue() {
            Node* node = head.load();
            while(node)
            {
                Node* next = node->next;
                delete node;
                node = next;
            }
            node = node_pool;
            for(int i=0;i<pool_size;i++)
            {
                Node* next = node->next;
                delete node;
                node = next;
            }
        }

        safe_queue(const safe_queue&) = delete;
        safe_queue& operator=(const safe_queue&) = delete;

        void push(const T& data) {
            Node* newNode = acquire_node();
            newNode->data = data;
            newNode->next = nullptr;
            Node* prevTail = tail.exchange(newNode);
            prevTail->next = newNode;
        }

        bool pop(T& result) {
            Node* prevHead = head.load(std::memory_order_acquire);
            Node* newHead;

            do {
                if(prevHead->next == nullptr) {
                    result = nullptr;
                    return false;
                }
                newHead = prevHead->next;
            } while(!head.compare_exchange_strong(prevHead,newHead));

            result = newHead->data;
            release_node(prevHead);
            return true;
        }

        bool empty() const
        {
            Node* head_ = head.load(std::memory_order_acquire);
            Node* tail_ = tail.load(std::memory_order_acquire);
            return head_ == tail_;
        }
    };
    template<>
    bool safe_queue<int>::pop(int& result)
    {
        Node* prevHead = head.load(std::memory_order_acquire);
        Node* newHead;

        do {
            if(prevHead->next == nullptr) {
                result = -1;
                return false;
            }
            newHead = prevHead->next;
        } while(!head.compare_exchange_strong(prevHead,newHead));

        result = newHead->data;
        release_node(prevHead);
        return true;
    }
}

#endif
