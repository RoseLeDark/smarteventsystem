#pragma once
#include "message.h"
#include <mutex>
#include <chrono>
#include "sorted_vector.h"
#include "timed_lock.h"

namespace ses {

    class SES_API eventmanager {
    public:
        using message_ptr = std::shared_ptr<message>;
        using id_type = typename message::id_type;

        eventmanager(uint64_t timedWaitMax);

        void postMessage(message_ptr msg, uint64_t maxWaitTime);
        void clearMessages();

        size_t      get_messages() const;
        message_ptr get_byID(id_type id, uint64_t maxTime);


        bool beginMessages();
        bool processMessages(int from, int to);
        bool processMessages(uint8_t prio);
        bool endProcessMessages();
    protected:
        void discardMessage(const message_ptr& msg);
    private:
        sorted_vector<message_ptr> m_vecMessages;
        std::vector<message_ptr> m_vecDiscards;
        timed_countlock m_ctLock;
    };
}

