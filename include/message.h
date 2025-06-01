#pragma once

#include "config.h"
#include <stdint.h>
#include <message.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "tool.h"

namespace ses {
    /// <summary>
    /// Die Klasse "message" repräsentiert eine Nachricht mit Zeitstempel, Priorität, Lebensdauer und eindeutiger ID. Sie bietet Methoden zur Verwaltung und Verarbeitung von Nachrichten, einschließlich Ablaufprüfung, Verwerfungszähler und Prioritätssteuerung.
    /// </summary>
    class SES_API message {
    public:
        /// <summary>
        /// message id
        /// </summary>
        struct SES_API id {
            union {
                struct {
                    /// <summary>
                    /// Wenn die Nachricht von intern kommt dann ist der  Wert 1 und von user dann 0
                    /// </summary>
                    uint8_t msg : 1; // 0 = extern, 1 = systemintern
                    /// <summary>
                    /// Wenn 1 dann kommt message aus einer Gruppe
                    /// </summary>
                    uint8_t gr : 1;  // 1 = Gruppe, 0 = Einzel
                    /// <summary>
                    /// Die reale ID
                    /// </summary>
                    uint32_t rid : 30;
                };
                /// <summary>
                /// Díe rawid der nachruht
                /// </summary>
                uint32_t full;

            };
            /// <summary>
            /// Konstruiert ein id-Objekt mit einem optionalen Rohwert.
            /// </summary>
            /// <param name="raw">Der Rohwert, der zur Initialisierung verwendet wird (Standardwert ist 0).</param>
            explicit id(uint32_t raw = 0) : rid(raw) {}

            /// <summary>
            /// Prüft, ob die Nachricht intern ist.
            /// </summary>
            /// <returns>Gibt true zurück, wenn msg gleich 1 ist, andernfalls false.</returns>
            bool is_internal() const { return msg == 1; }
            /// <summary>
            /// Prüft, ob es sich um eine Gruppe handelt.
            /// </summary>
            /// <returns>Gibt true zurück, wenn gr gleich 1 ist, andernfalls false.</returns>
            bool is_group() const { return gr == 1; }
            /// <summary>
            /// Gibt die rohe ID zurück.
            /// </summary>
            /// <returns>Die gespeicherte Roh-ID als uint32_t.</returns>
            uint32_t raw_id() const { return rid; }

        };
        friend class eventmanager;

        /// <summary>
        /// Definiert einen Aliasnamen für den Typ 'id'.
        /// </summary>
        using id_type = id;
		/// <summary>
		/// Definiert einen Alias für einen gemeinsam genutzten Zeiger auf ein message-Objekt.
		/// </summary>
		using pointer_type = message*;

        /// <summary>
        /// Konstruiert ein message-Objekt mit Standardwerten.
        /// </summary>
        message() : message(5, 0) { }
		/// <summary>
		/// Konstruiert ein message-Objekt mit Priorität, Zeitstempel und optionalen Einstellungen für System- und Gruppennachrichten.
		/// </summary>
		/// <param name="prio">Die Priorität der Nachricht als 8-Bit-Ganzzahl.</param>
		/// <param name="ms">Die Lebensdauer der Nachricht in Millisekunden (Standardwert: 1000).</param>
		/// <param name="bIsSystem">Gibt an, ob es sich um eine Systemnachricht handelt (Standardwert: false).</param>
		/// <param name="bIsGroup">Gibt an, ob es sich um eine Gruppennachricht handelt (Standardwert: false).</param>
		message(uint8_t prio, uint32_t ms = 1000, bool bIsSystem = false, bool bIsGroup = false) 
            : m_iCount(0), m_uiTimeStamp(tool::now()), m_uiAliveMs(ms), m_ucPriority(prio), m_id(message::get_nextid(bIsSystem, bIsGroup) ) , m_iMaxCount(5){ }

		message(const message& other) = default;
		message(message&& other) = default;
        virtual ~message() {}

        /// <summary>
        /// Wird aufgerufen, nachdem eine Nachricht gepostet wurde.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das die Nachricht gesendet hat.</param>
        /// <param name="bWasAdd">Gibt an, ob die Nachricht hinzugefügt (true) würde oder fehler passierte zb Zeit abgelafen (false).</param>
        virtual void onMessagePost(void* sender, bool bWasAdd) = 0;
        /// <summary>
        /// Wird aufgerufen, um eine Nachricht zu verarbeiten.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das die Nachricht gesendet hat.</param>
        /// <returns>Gibt true zurück, wenn die Nachricht erfolgreich verarbeitet wurde, andernfalls false.</returns>
        virtual bool onMessageProcess(void* sender) = 0;
        /// <summary>
        /// Wird aufgerufen, wenn eine Nachricht verworfen wird.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das das Ereignis ausgelöst hat.</param>
        /// <param name="time">Der Zeitstempel (in Mikrosekunden), zu dem die Nachricht verworfen wurde.</param>
        virtual void onMessageDiscard(void* sender, uint64_t time) = 0;
        /// <summary>
        /// Wird aufgerufen, wenn eine Nachricht abgelaufen ist.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das das Ereignis ausgelöst hat.</param>
        /// <param name="time">Der Zeitpunkt (in Millisekunden seit Epoche), zu dem die Nachricht abgelaufen ist.</param>
        virtual void onMessageExpired(void* sender, uint64_t time) = 0;
        
        /// <summary>
        /// Prüft, ob ein Objekt abgelaufen ist.
        /// </summary>
        /// <param name="now">Der aktuelle Zeitstempel in Millisekunden.</param>
        /// <returns>Gibt true zurück, wenn das Objekt abgelaufen ist, andernfalls false.</returns>
        virtual bool is_expired(uint64_t now) const {
            return m_uiAliveMs > 0 && now > (m_uiTimeStamp + m_uiAliveMs);
        }
        /// <summary>
        /// Prüft, ob die maximale Anzahl erreicht oder überschritten wurde.
        /// </summary>
        /// <returns>Gibt true zurück, wenn m_iCount größer oder gleich m_iMaxCount ist, andernfalls false.</returns>
        bool is_maxDiscard() const { 
            return m_iCount >= m_iMaxCount; 
        }
        /// <summary>
        /// Gibt den Zeitstempel zurück.
        /// </summary>
        /// <returns>Der aktuelle Zeitstempel als uint64_t-Wert.</returns>
        uint64_t get_timestamp() const { return m_uiTimeStamp; }
        /// <summary>
        /// Gibt die Anzahl wie oft die Nachricht verworfen würde
        /// </summary>
        /// <returns>Die Anzahl wie oft die Nachricht verworfen würde als uint32_t.</returns>
        uint32_t get_discards() const { return m_iCount; }
        /// <summary>
        /// Gibt die Anzahl der Millisekunden zurück, wie lange díe Nachrcht lebt
        /// </summary>
        /// <returns>Die Anzahl der Millisekunden, die das Objekt aktiv ist (als uint32_t).</returns>
        uint32_t get_alivems() const { return m_uiAliveMs; }
        /// <summary>
        /// Gibt die Priorität zurück.
        /// </summary>
        /// <returns>Die Priorität als uint8_t-Wert.</returns>
        uint8_t  get_priority() const { return m_ucPriority; }
        /// <summary>
        /// Gibt die aktuelle ID zurück.
        /// </summary>
        /// <returns>Die aktuelle ID des Objekts.</returns>
        id_type  get_id() const { return m_id; }

		/// <summary>
		/// Setzt den Zeitstempel auf den angegebenen Wert.
		/// </summary>
		/// <param name="ts">Der neue Zeitstempelwert, der gesetzt werden soll.</param>
		void set_timestamp(uint64_t ts) { m_uiTimeStamp = ts; }
		/// <summary>
		/// Setzt die Alive-Zeit in Millisekunden.
		/// </summary>
		/// <param name="ms">Die Anzahl der Millisekunden, die als Alive-Zeit gesetzt werden soll.</param>
		void set_alivems(uint32_t ms) { m_uiAliveMs = ms; }
        /// <summary>
        /// Setzt die Priorität auf den angegebenen Wert.
        /// </summary>
        /// <param name="priority">Der zu setzende Prioritätswert.</param>
        void set_priority(uint8_t priority) { m_ucPriority = priority; }
        /// <summary>
        /// Setzt die maximale Anzahl wie oft diese Nachricht werworfen werden darf, bis sie als abgelaufen gilt.
        /// </summary>
        /// <param name="max">Die maximale Anzahl der zu verwerfenden Elemente.</param>
        void set_maxdiscards(uint8_t max) { m_iMaxCount = max;  }
        /// <summary>
        /// Erhöht den internen Zähler um eins, um einen Verwerfungszustand zu setzen.
        /// </summary>
        void set_discard() {  m_iCount++;  }

        
        /// <summary>
        /// Kopiert die Werte aller Membervariablen aus einer anderen message-Instanz in dieses Objekt.
        /// </summary>
        /// <param name="other">Die message-Instanz, deren Werte kopiert werden sollen.</param>
        /// <returns>Eine Referenz auf dieses message-Objekt nach der Zuweisung.</returns>
        message& operator=(const message& other) {
            if (this != &other) {
                m_iCount = other.m_iCount;
                m_uiTimeStamp = other.m_uiTimeStamp;
                m_uiAliveMs = other.m_uiAliveMs;
                m_ucPriority = other.m_ucPriority;
                m_id = other.m_id;
            }
            return *this;
        }
        /// <summary>
        /// Vergleicht zwei message-Objekte auf Gleichheit anhand ihrer m_id.full-Werte.
        /// </summary>
        /// <param name="other">Das message-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn beide message-Objekte denselben m_id.full-Wert haben, andernfalls false.</returns>
        bool operator==(const message& other) const {
            return m_id.full == other.m_id.full;
		}
        /// <summary>
        /// Prüft, ob zwei message-Objekte ungleich sind.
        /// </summary>
        /// <param name="other">Das message-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die beiden message-Objekte nicht gleich sind, andernfalls false.</returns>
        bool operator!=(const message& other) const {
            return !(*this == other);
        }
        /// <summary>
        /// Vergleicht zwei message-Objekte anhand ihrer m_id.full-Werte.
        /// </summary>
        /// <param name="other">Das message-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn das aktuelle message-Objekt einen kleineren m_id.full-Wert als other hat, andernfalls false.</returns>
        bool operator<(const message& other) const {
            return m_id.full < other.m_id.full;
		}
        /// <summary>
        /// Vergleicht, ob die ID dieser Nachricht größer als die ID einer anderen Nachricht ist.
        /// </summary>
        /// <param name="other">Die andere Nachricht, mit der verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die ID dieser Nachricht größer als die der anderen Nachricht ist, andernfalls false.</returns>
        bool operator>(const message& other) const {
            return m_id.full > other.m_id.full;
        }
        /// <summary>
        /// Vergleicht, ob die aktuelle Nachricht kleiner oder gleich einer anderen Nachricht ist.
        /// </summary>
        /// <param name="other">Die andere Nachricht, mit der verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die aktuelle Nachricht kleiner oder gleich der anderen Nachricht ist, andernfalls false.</returns>
        bool operator<=(const message& other) const {
            return m_id.full <= other.m_id.full;
        }
        /// <summary>
        /// Vergleicht, ob die aktuelle Nachricht größer oder gleich einer anderen Nachricht ist.
        /// </summary>
        /// <param name="other">Die andere Nachricht, mit der verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die aktuelle Nachricht größer oder gleich der anderen Nachricht ist, andernfalls false.</returns>
        bool operator>=(const message& other) const {
            return m_id.full >= other.m_id.full;
		}
        /// <summary>
        /// Vergleicht, ob zwei id_type-Objekte gleich sind.
        /// </summary>
        /// <param name="other">Das andere id_type-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn beide id_type-Objekte gleich sind, andernfalls false.</returns>
        bool operator==(const id_type& other) const {
            return m_id.full == other.full;
        }
        /// <summary>
        /// Prüft, ob zwei id_type-Objekte ungleich sind.
        /// </summary>
        /// <param name="other">Das zu vergleichende id_type-Objekt.</param>
        /// <returns>Gibt true zurück, wenn die beiden Objekte ungleich sind, andernfalls false.</returns>
        bool operator!=(const id_type& other) const {
            return !(*this == other);
        }
        /// <summary>
        /// Vergleicht zwei id_type-Objekte auf Grundlage ihres full-Mitglieds.
        /// </summary>
        /// <param name="other">Das id_type-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn das full-Mitglied dieses Objekts kleiner als das von other ist, andernfalls false.</returns>
        bool operator<(const id_type& other) const {
            return m_id.full < other.full;
        }
        /// <summary>
        /// Vergleicht, ob die aktuelle id_type-Instanz größer als eine andere ist.
        /// </summary>
        /// <param name="other">Die andere id_type-Instanz, mit der verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die aktuelle Instanz größer als other ist, andernfalls false.</returns>
        bool operator>(const id_type& other) const {
            return m_id.full > other.full;
        }
        /// <summary>
        /// Vergleicht, ob die aktuelle id_type-Instanz kleiner oder gleich einer anderen ist.
        /// </summary>
        /// <param name="other">Das andere id_type-Objekt, mit dem verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die aktuelle Instanz kleiner oder gleich other ist, andernfalls false.</returns>
        bool operator<=(const id_type& other) const {
            return m_id.full <= other.full;
        }
        /// <summary>
        /// Vergleicht, ob die aktuelle id_type-Instanz größer oder gleich einer anderen ist.
        /// </summary>
        /// <param name="other">Die andere id_type-Instanz, mit der verglichen wird.</param>
        /// <returns>Gibt true zurück, wenn die aktuelle Instanz größer oder gleich der anderen ist, andernfalls false.</returns>
        bool operator>=(const id_type& other) const {
            return m_id.full >= other.full;
		}
        void set_runned() { m_bMarked = true; }
        bool is_marked() { return m_bMarked; }
    private:
        /// <summary>
        /// Gibt die nächste eindeutige ID zurück.
        /// </summary>
        /// <returns>Die nächste gültige ID vom Typ id_type.</returns>
        static id_type get_nextid(bool bIsIntern, bool bIsGroup = false) {
            static uint32_t next_id =  0 ; // Starte bei 1, 0 = ungültig
            
            id_type _ret =  id(next_id++);

            _ret.msg = (bIsIntern) ? 1 : 0;
            _ret.gr = (bIsGroup) ? 1 : 0;
           
            return _ret;
        }
    protected:
        uint8_t m_iCount;
        uint64_t m_uiTimeStamp; // Zeitpunkt des Sendens
        uint32_t m_uiAliveMs; // Gültigkeit
        uint8_t  m_ucPriority; // 0 = höchste Priorität
		id_type m_id; // ID des Messages
        uint8_t m_iMaxCount;
        bool m_bMarked;
    };

    /// <summary>
    /// Die Klasse message_group verwaltet eine Gruppe von Nachrichtenobjekten und leitet Nachrichtenereignisse an alle gespeicherten Nachrichten weiter.
    /// </summary>
    class SES_API message_group : public message {
    public:
        using message_type = typename message::pointer_type;

		/// <summary>
		/// Konstruiert ein message_group-Objekt und ruft den Konstruktor der Basisklasse message mit dem Wert 5 auf.
		/// </summary>
		message_group() : message(5) {}
        /// <summary>
        /// Konstruiert ein message_group-Objekt mit den angegebenen Parametern.
        /// </summary>
        /// <param name="prio">Die Priorität der Nachricht.</param>
        /// <param name="ms">Die gultikeit der nachricht (Standardwert ist 1000 ms).</param>
        /// <param name="bIsSystem">Gibt an, ob es sich um eine Systemnachricht handelt (Standardwert ist false).</param>
        message_group(uint8_t prio, uint32_t ms = 1000, bool bIsSystem = false) : message(prio, ms, bIsSystem, true) {   }

		virtual ~message_group() {}
        virtual std::string source() const = 0;

        void addSubMessage(const message_type msg) {
            m_ptrMessages.push_back(msg);
        }
        /// <summary>
        /// Wird aufgerufen, nachdem eine Nachricht gepostet wurde, und benachrichtigt alle gespeicherten Nachrichtenobjekte.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das die Nachricht gesendet hat.</param>
        /// <param name="bWasAdd">Gibt an, ob die Nachricht hinzugefügt (true) oder entfernt (false) wurde.</param>
        virtual void onMessagePost(void* sender, bool bWasAdd) {
            for (auto& var : m_ptrMessages) {  
                var->onMessagePost(sender, bWasAdd);  
            }
        }
        /// <summary>
        /// Verarbeitet Nachrichten, indem sie an alle gespeicherten Nachrichtenobjekte weitergeleitet werden.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das die Nachricht sendet.</param>
        /// <returns>Gibt true zurück, wenn die Verarbeitung erfolgreich war; andernfalls false.</returns>
        virtual bool onMessageProcess(void* sender) {
            for (auto& var : m_ptrMessages) {
                var->onMessageProcess(sender);
            }
            return true;
        }
        /// <summary>
        /// Wird aufgerufen, wenn eine Nachricht verworfen wird, und benachrichtigt alle gespeicherten Nachrichtenobjekte darüber.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das das Ereignis ausgelöst hat.</param>
        /// <param name="time">Der Zeitstempel (in Mikrosekunden), zu dem die Nachricht verworfen wurde.</param>
        virtual void onMessageDiscard(void* sender, uint64_t time) {
            for (auto& var : m_ptrMessages) {
                var->onMessageDiscard(sender, time);
            }
        }
        /// <summary>
        /// Wird aufgerufen, wenn eine Nachricht abgelaufen ist, und benachrichtigt alle gespeicherten Nachrichtenobjekte darüber.
        /// </summary>
        /// <param name="sender">Ein Zeiger auf das Objekt, das das Ereignis ausgelöst hat.</param>
        /// <param name="time">Der Zeitpunkt (als 64-Bit-Ganzzahl), zu dem die Nachricht abgelaufen ist.</param>
        virtual void onMessageExpired(void* sender, uint64_t time) {
            for (auto& var : m_ptrMessages) {
                var->onMessageExpired(sender, time);
            }
        }
        /// <summary>
        /// Gibt die Anzahl der Nachrichten zurück.
        /// </summary>
        /// <returns>Die Anzahl der Nachrichten als size_t.</returns>
        size_t get_cound() {
            return m_ptrMessages.size();
        }

        /// <summary>
        /// Löscht alle Nachrichten aus der Sammlung.
        /// </summary>
        void clear() {
            m_ptrMessages.clear();
        }

    protected:
        /// <summary>
        /// Ein Vektor, der Nachrichtenobjekte speichert.
        /// </summary>
        std::vector<message_type> m_ptrMessages;
    };


}