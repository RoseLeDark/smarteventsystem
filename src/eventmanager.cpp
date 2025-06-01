#include "eventmanager.h"
#include "sorted_vector.h" // Ensure the correct header for sorted_vector is included

namespace ses {
    static bool compare_message(const eventmanager::message_ptr& a, const eventmanager::message_ptr& b) {
        return a->get_priority() < b->get_priority(); // kleiner = höhere Priorität
    }

    eventmanager::eventmanager(uint64_t timedWaitMax)
        : m_vecMessages(true, compare_message), m_ctLock(timedWaitMax) // Autosort enabled
    {
    }

    void eventmanager::postMessage(message_ptr msg, uint64_t maxWaitTime) {
        if (m_ctLock.try_lock(maxWaitTime)) 
        {
            msg->onMessagePost(this, true);
            m_vecMessages.push_back(msg);
            m_ctLock.release();  // Lock wieder freigeben!
        }
        else 
        {
            msg->onMessagePost(this, false);
        }
    }

    void eventmanager::clearMessages() {
        if (m_ctLock.try_lock(TIMEDLOCK_INFINITY_WAIT)) {
            m_vecMessages.clear();
            m_vecDiscards.clear();
            m_ctLock.release();  // Lock wieder freigeben!
        }
    }

    size_t eventmanager::get_messages() const {
        return m_vecMessages.size();
    }

    eventmanager::message_ptr eventmanager::get_byID(id_type id, uint64_t maxTime) {
        if (m_ctLock.try_lock(maxTime)) {
            auto it = std::find_if(m_vecMessages.begin(), m_vecMessages.end(),
                [id](const message_ptr& msg) { return msg->get_id().full == id.full; });
            return (it != m_vecMessages.end()) ? *it : nullptr;
        }
        return nullptr;
    }

    bool eventmanager::beginMessages() {
        m_ctLock.add();
        size_t size = m_vecMessages.size();

        if (size > 0) {
            std::cout << "messgae size: " << m_vecMessages.size() << " locks: " << m_ctLock.get_locks()  << " \n";
        }
        return true;
        
    }
    bool eventmanager::endProcessMessages() {
        for (auto it = m_vecMessages.begin(); it != m_vecMessages.end(); )
        {
            message_ptr& msg = *it;
            if (msg->is_marked() == true)
                it = m_vecMessages.remove(it);
            else
                ++it;
        }
        m_ctLock.release();

        std::cout << "messgae size: " << m_vecMessages.size() << " locks: " << m_ctLock.get_locks() << " \n";
        
        return true;
    }
    bool eventmanager::processMessages(int from, int to) {
        if (m_ctLock.get_locks() == 0) return false;
        uint64_t now = tool::now();

        for (auto it = m_vecMessages.begin(); it != m_vecMessages.end(); it++ ) {
            message_ptr& msg = *it;
            if (msg == 0) continue;

            if (msg->is_expired(now)) {
                msg->onMessageExpired(this, now);
                msg->set_runned();
            }
            else {
                int prio = msg->get_priority();
                if (prio >= from && prio <= to && msg->is_marked() == false) {
                 
                    if (msg->onMessageProcess(this))
                        msg->set_runned();
                    else
                        discardMessage(msg);
                }
            }
        }
        return true;
    }

    bool eventmanager::processMessages(uint8_t prio) {
        return processMessages(prio, prio);
    }

    void eventmanager::discardMessage(const message_ptr& msg) {

        msg->set_discard();
        if (msg->get_discards() >= 5) {
            m_vecDiscards.push_back(msg);
            m_vecMessages.remove(msg);

            msg->onMessageDiscard(this, tool::now());
        }
    }
}