#pragma once

#include "tool.h"
#include <atomic>
#include <chrono>
#include <thread>
#include "tool.h"
#include <mutex>


namespace ses {

    /// <summary>
    /// Die Klasse timed_countlock implementiert einen Zähllock mit Timeout-Funktionalität, der es ermöglicht, Sperren mit einer maximalen Wartezeit zu verwalten.
    /// </summary>
    class SES_API timed_countlock {
    public:
        /// <summary>
        /// Erzeugt ein timed_countlock-Objekt mit einer angegebenen Timeout-Dauer in Millisekunden.
        /// </summary>
        /// <param name="ms">Die Timeout-Dauer in Millisekunden.</param>
        timed_countlock(uint64_t ms) : m_ulTimeOut(ms), m_ulLastTime(0), m_iLocks(0) {  }

        /// <summary>
        /// Erhöht den Sperrzähler atomar und aktualisiert die Zeit des letzten Zugriffs.
        /// </summary>
        void add() {
            const std::lock_guard<std::mutex> lock(m_ms);

            m_iLocks++;
            m_ulLastTime = tool::now();
        }
        /// <summary>
        /// Gibt das Sperrobjekt frei und prüft, ob keine Sperren mehr vorhanden sind.
        /// </summary>
        /// <returns>Gibt true zurück, wenn nach dem Freigeben keine Sperren mehr vorhanden sind, andernfalls false.</returns>
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
        /// <returns>Gibt true zurück, wenn das Schloss erfolgreich innerhalb der Wartezeit erworben wurde, andernfalls false.</returns>
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

                std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Kurzes Warten vor nächster Prüfung
            }
        }

        uint32_t get_locks() const { return m_iLocks; }
    private:
        
        /// <summary>
        /// Überprüft, ob ein Timeout überschritten wurde, und dekrementiert gegebenenfalls einen Sperrzähler.
        /// </summary>
        void check_timeout() {
            if (m_iLocks == 0) return;

            uint64_t now = tool::now();
            uint64_t last = m_ulLastTime;
            if (m_iLocks > 0 && now > last + m_ulTimeOut) {
                // Timeout überschritten, Counter dekrementieren
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