#pragma once

#include "tool.h"
#include <atomic>
#include <chrono>
#include <thread>
#include "tool.h"
#include <mutex>


namespace ses {

    /// <summary>
    /// Die Klasse timed_countlock implementiert einen Z�hllock mit Timeout-Funktionalit�t, der es erm�glicht, Sperren mit einer maximalen Wartezeit zu verwalten.
    /// </summary>
    class SES_API timed_countlock {
    public:
        /// <summary>
        /// Erzeugt ein timed_countlock-Objekt mit einer angegebenen Timeout-Dauer in Millisekunden.
        /// </summary>
        /// <param name="ms">Die Timeout-Dauer in Millisekunden.</param>
        timed_countlock(uint64_t ms) : m_ulTimeOut(ms), m_ulLastTime(0), m_iLocks(0) {  }

        /// <summary>
        /// Erh�ht den Sperrz�hler atomar und aktualisiert die Zeit des letzten Zugriffs.
        /// </summary>
        void add() {
            const std::lock_guard<std::mutex> lock(m_ms);

            m_iLocks++;
            m_ulLastTime = tool::now();
        }
        /// <summary>
        /// Gibt das Sperrobjekt frei und pr�ft, ob keine Sperren mehr vorhanden sind.
        /// </summary>
        /// <returns>Gibt true zur�ck, wenn nach dem Freigeben keine Sperren mehr vorhanden sind, andernfalls false.</returns>
        bool release() {
            const std::lock_guard<std::mutex> lock(m_ms);

            if (m_iLocks == 0) return false;
            m_iLocks--;
            return m_iLocks == 0;
        }

        /// <summary>
        /// Versucht, ein Schloss innerhalb einer maximalen Wartezeit zu erwerben.
        /// </summary>
        /// <param name="max_wait_ms">Die maximale Wartezeit in Millisekunden, um das Schloss zu erwerben.</param>
        /// <returns>Gibt true zur�ck, wenn das Schloss erfolgreich innerhalb der Wartezeit erworben wurde, andernfalls false.</returns>
        bool try_lock(uint64_t max_wait_ms) {
            const std::lock_guard<std::mutex> lock(m_ms);

            uint64_t start = tool::now();
            while (true) {
                check_timeout();

                if (m_iLocks == 0) {
                    m_iLocks++;
                    m_ulLastTime = tool::now();
                    return true;
                }
                

                if (max_wait_ms != TIMEDLOCK_INFINITY_WAIT && tool::now() - start > max_wait_ms) {
                    return false;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Kurzes Warten vor n�chster Pr�fung
            }
        }

        uint32_t get_locks() const { return m_iLocks; }
    private:
        
        /// <summary>
        /// �berpr�ft, ob ein Timeout �berschritten wurde, und dekrementiert gegebenenfalls einen Sperrz�hler.
        /// </summary>
        void check_timeout() {
            if (m_iLocks == 0) return;

            uint64_t now = tool::now();
            uint64_t last = m_ulLastTime;
            if (m_iLocks > 0 && now > last + m_ulTimeOut) {
                // Timeout �berschritten, Counter dekrementieren
                m_iLocks--;
                m_ulLastTime = now;
            }
        }
    private:
        std::mutex m_ms;
        volatile  uint32_t m_iLocks;
        const uint64_t m_ulTimeOut;
        uint64_t m_ulLastTime;
    };
}