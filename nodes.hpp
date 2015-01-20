#ifndef NODES_HPP
#define NODES_HPP

#include <algorithm>
#include <vector>
#include <tuple>

namespace BwTree {
    using PID = std::size_t;
    constexpr PID NotExistantPID = std::numeric_limits<PID>::max();

    enum class PageType : std::int8_t {
        leaf,
        inner,
        deltaInsert,
        deltaDelete,
        deltaIndex,
        deltaSplit,
        deltaSplitInner
    };

    template<typename Key, typename Data>
    struct Node {
        PageType type;
    };


    template<typename Key, typename Data>
    struct LinkedNode : Node<Key, Data> {
        PID prev;
        PID next;
    };

    template<typename Key, typename Data>
    struct Leaf : LinkedNode<Key, Data> {
        std::size_t recordCount;
        // has to be last member for dynamic operator new() !!!
        std::tuple<Key, const Data *> records[];
    private:
        Leaf() = delete;

        ~Leaf() = delete;
    };


    template<typename Key, typename Data>
    struct InnerNode : LinkedNode<Key, Data> {
        std::size_t nodeCount;
        // has to be last member for dynamic operator new() !!!
        std::tuple<Key, PID> nodes[];
    private:
        InnerNode() = delete;

        ~InnerNode() = delete;
    };

    template<typename Key, typename Data>
    struct DeltaNode : Node<Key, Data> {
        Node<Key, Data> *origin;
    private:
        DeltaNode() = delete;

        ~DeltaNode() = delete;
    };

    template<typename Key, typename Data>
    struct DeltaInsert : DeltaNode<Key, Data> {
        std::tuple<Key, const Data *> record;
        bool keyExistedBefore;
    private:
        DeltaInsert() = delete;

        ~DeltaInsert() = delete;
    };

    template<typename Key, typename Data>
    struct DeltaDelete : DeltaNode<Key, Data> {
        Key key;
    private:
        DeltaDelete() = delete;

        ~DeltaDelete() = delete;
    };

    template<typename Key, typename Data>
    struct DeltaSplit : DeltaNode<Key, Data> {
        Key key;
        PID sidelink;
        std::size_t removedElements;
    private:
        DeltaSplit() = delete;

        ~DeltaSplit() = delete;
    };

    template<typename Key, typename Data>
    struct DeltaIndex : DeltaNode<Key, Data> {
        Key keyLeft; // greater than
        Key keyRight; // less or equal than
        PID child;
        PID oldChild;
    private:
        DeltaIndex() = delete;

        ~DeltaIndex() = delete;
    };

    template<typename Key, typename Data>
    InnerNode<Key, Data> *CreateInnerNode(std::size_t size, const PID &next, const PID &prev) {
        size_t s = sizeof(InnerNode<Key, Data>) - sizeof(InnerNode<Key, Data>::nodes);
        InnerNode<Key, Data> *output = (InnerNode<Key, Data> *) operator new(s + size * sizeof(std::tuple<Key, PID>));
        output->nodeCount = size;
        output->type = PageType::inner;
        output->next = next;
        output->prev = prev;
        return output;
    }

    template<typename Key, typename Data>
    Leaf<Key, Data> *CreateLeaf(std::size_t size, const PID &next, const PID &prev) {
        size_t s = sizeof(Leaf<Key, Data>) - sizeof(Leaf<Key, Data>::records);
        Leaf<Key, Data> *output = (Leaf<Key, Data> *) operator new(s + size * sizeof(std::tuple<Key, const Data *>));
        output->recordCount = size;
        output->type = PageType::leaf;
        output->next = next;
        output->prev = prev;
        return output;
    }

    template<typename Key, typename Data>
    class Helper {


        //InnerNode<Key, Data> *CreateInnerNodeFromUnsorted(std::vector<std::tuple<Key, PID>>::const_iterator begin, std::vector<std::tuple<Key, PID>>::const_iterator end, const PID &next, const PID &prev, bool infinityElement);

        //template<typename Key, typename Data>
    public:
        typedef typename std::vector<std::tuple<Key, PID>>::iterator InnerIterator;

        static InnerNode<Key, Data> *CreateInnerNodeFromUnsorted(InnerIterator begin, InnerIterator end, const PID &next, const PID &prev, bool infinityElement) {
            // construct a new node
            auto newNode = CreateInnerNode<Key, Data>(std::distance(begin, end), next, prev);
            std::sort(begin, end, [](const std::tuple<Key, PID> &t1, const std::tuple<Key, PID> &t2) {
                return std::get<0>(t1) < std::get<0>(t2);
            });
            int i = 0;
            for (auto it = begin; it != end; ++it) {
                newNode->nodes[i++] = *it;
            }
            if (infinityElement) {
                std::get<0>(newNode->nodes[newNode->nodeCount - 1]) = std::numeric_limits<Key>::max();
            }
            return newNode;
        }

        typedef typename std::vector<std::tuple<Key, const Data *>>::iterator LeafIterator;

        static Leaf<Key, Data> *CreateLeafNodeFromUnsorted(LeafIterator begin, LeafIterator end, const PID &next, const PID &prev) {
            // construct a new node
            auto newNode = CreateLeaf<Key, Data>(std::distance(begin, end), next, prev);
            std::sort(begin, end, [](const std::tuple<Key, const Data *> &t1, const std::tuple<Key, const Data *> &t2) {
                return std::get<0>(t1) < std::get<0>(t2);
            });
            int i = 0;
            for (auto it = begin; it != end; ++it) {
                newNode->records[i++] = *it;
            }
            return newNode;
        }
    };

    template<typename Key, typename Data>
    DeltaInsert<Key, Data> *CreateDeltaInsert(Node<Key, Data> *origin, std::tuple<Key, const Data *> record, bool keyExistedBefore) {
        size_t s = sizeof(DeltaInsert<Key, Data>);
        DeltaInsert<Key, Data> *output = (DeltaInsert<Key, Data> *) operator new(s);
        output->type = PageType::deltaInsert;
        output->origin = origin;
        output->record = record;
        output->keyExistedBefore = keyExistedBefore;
        return output;
    }

    template<typename Key, typename Data>
    DeltaDelete<Key, Data> *CreateDeltaDelete(Node<Key, Data> *origin, Key key) {
        size_t s = sizeof(DeltaDelete<Key, Data>);
        DeltaDelete<Key, Data> *output = (DeltaDelete<Key, Data> *) operator new(s);
        output->type = PageType::deltaDelete;
        output->origin = origin;
        output->key = key;
        return output;
    }

    template<typename Key, typename Data>
    DeltaSplit<Key, Data> *CreateDeltaSplit(Node<Key, Data> *origin, Key splitKey, PID sidelink, std::size_t removedElements) {
        size_t s = sizeof(DeltaSplit<Key, Data>);
        DeltaSplit<Key, Data> *output = (DeltaSplit<Key, Data> *) operator new(s);
        output->type = PageType::deltaSplit;
        output->origin = origin;
        output->key = splitKey;
        output->sidelink = sidelink;
        output->removedElements = removedElements;
        return output;
    }

    template<typename Key, typename Data>
    DeltaSplit<Key, Data> *CreateDeltaSplitInner(Node<Key, Data> *origin, Key splitKey, PID sidelink, std::size_t removedElements) {
        DeltaSplit<Key, Data> *output = CreateDeltaSplit(origin, splitKey, sidelink, removedElements);
        output->type = PageType::deltaSplitInner;
        return output;
    }

    template<typename Key, typename Data>
    DeltaIndex<Key, Data> *CreateDeltaIndex(Node<Key, Data> *origin, Key splitKeyLeft, Key splitKeyRight, PID child, PID oldChild) {
        size_t s = sizeof(DeltaIndex<Key, Data>);
        DeltaIndex<Key, Data> *output = (DeltaIndex<Key, Data> *) operator new(s);
        output->type = PageType::deltaIndex;
        output->origin = origin;
        output->keyLeft = splitKeyLeft;
        output->keyRight = splitKeyRight;
        output->child = child;
        output->oldChild = oldChild;
        return output;
    }

    template<typename Key, typename Data>
    void freeNodeRecursively(Node<Key, Data> *node);

    template<typename Key, typename Data>
    void freeNodeSingle(Node<Key, Data> *node) {
        operator delete(node);
    }

    template<typename Key, typename Data>
    void freeNodeRecursively(Node<Key, Data> *node) {
        while (node != nullptr) {
            switch (node->type) {
                case PageType::inner: /* fallthrough */
                case PageType::leaf: {
                    freeNodeSingle<Key, Data>(node);
                    return;
                }
                case PageType::deltaIndex: /* fallthrough */
                case PageType::deltaDelete: /* fallthrough */
                case PageType::deltaSplitInner: /* fallthrough */
                case PageType::deltaSplit: /* fallthrough */
                case PageType::deltaInsert: {
                    auto node1 = static_cast<DeltaNode<Key, Data> *>(node);
                    node = node1->origin;
                    freeNodeSingle<Key, Data>(node1);
                    continue;
                }
                default: {
                    assert(false);//all nodes have to be handeled
                }
            }
            node = nullptr;
        }
    }
}
#endif