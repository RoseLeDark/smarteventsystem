/**
* SES Smart Event System
* (c) 2025 Amber-Sophia Schröck
* This file is copyright under the latest version of the EUPL.
* Please see LICENSE file for your rights under this license.
**/
#pragma once
#include <vector>
#include <functional>
#include <algorithm>

#include "sorted.h"

namespace ses {
    /// <summary>
    /// Eine generische Container-Klasse, die einen sortierten Vektor mit optionaler automatischer Sortierung und benutzerdefinierbarer Vergleichsfunktion implementiert.
    /// </summary>
    /// <typeparam name="T">Der Typ der Elemente, die im sortierten Vektor gespeichert werden.</typeparam>
    /// <typeparam name="TAlloc">Der Typ des Allocators, der für die Speicherverwaltung verwendet wird (Standard: std::allocator<T>).</typeparam>
    template <class T>
    class sorted_vector : public sorted<T> {
    public:
        /// <summary>
        /// Definiert einen Aliasnamen 'base_type' für den Typ 'sorted'.
        /// </summary>
        using base_type = sorted<T>;
        /// <summary>
        /// Definiert einen Alias für den Vergleichsfunktionstyp von base_type.
        /// </summary>
        using compare_func_t = typename base_type::compare_func_t;

        /// <summary>
        /// Definiert einen Alias für einen std::vector mit dem angegebenen Element- und Allocator-Typ.
        /// </summary>
        using container_type = std::vector<T>;

        /// <summary>
        /// Definiert einen Alias für den Iterator-Typ eines std::vector mit Elementtyp T.
        /// </summary>
        using iterator = typename container_type::iterator;
        /// <summary>
        /// Definiert einen Typalias für den konstanten Iterator eines std::vector mit Elementen vom Typ T.
        /// </summary>
        using const_iterator = typename container_type::const_iterator;

        using value_type = typename container_type::value_type;
        using pointer = typename container_type::pointer;
        using const_pointer = typename container_type::const_pointer;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using size_type = typename container_type::size_type;
        using difference_type = typename container_type::difference_type;

        iterator        begin()         { return m_vecData.begin(); }
        iterator        end()           { return m_vecData.end(); }
        const_iterator  begin() const   { return m_vecData.begin(); }
        const_iterator  end()   const   { return m_vecData.end(); }

        /// <summary>
        /// Konstruiert ein sorted_vector-Objekt mit optionaler automatischer Sortierung.
        /// </summary>
        /// <param name="auto_sort">Legt fest, ob das sortierte Verhalten beim Einfügen von Elementen automatisch aktiviert wird. Standardmäßig auf true gesetzt.</param>
        sorted_vector(bool auto_sort = true)
            : base_type(auto_sort), m_funcCompare([](const T a, const T b) { return a < b; }) {
        }

        /// <summary>
        /// Konstruiert ein sorted_vector-Objekt mit optionaler automatischer Sortierung.
        /// </summary>
        /// <param name="auto_sort">Legt fest, ob das sortierte Verhalten beim Einfügen von Elementen automatisch aktiviert wird. Standardmäßig auf true gesetzt.</param>
        sorted_vector(bool auto_sort, compare_func_t func)
            : base_type(auto_sort), m_funcCompare(func) {
        }

        /// <summary>
        /// Setzt die Vergleichsfunktion und sortiert optional die Daten neu.
        /// </summary>
        /// <param name="comp">Die neue Vergleichsfunktion, die zum Sortieren der Daten verwendet wird.</param>
        void set_handle(compare_func_t comp) {
            m_funcCompare = std::move(comp);
            if (base_type::m_bAutosort) {
                std::sort(m_vecData.begin(), m_vecData.end(), m_funcCompare);
            }
        }

        /// <summary>
        /// Fügt ein Element am Ende der Sammlung hinzu.
        /// </summary>
        /// <param name="item">Das einzufügende Element.</param>
        void insert(const T& item) {
            push_back(item); 
		}

        /// <summary>
        /// Entfernt das angegebene Element aus der Sammlung, falls es vorhanden ist.
        /// </summary>
        /// <param name="item">Das zu entfernende Element.</param>
        /// <returns>Gibt true zurück, wenn das Element entfernt wurde, andernfalls false.</returns>
        bool remove(const T& item) {
            bool _ret = false;
            auto it = std::find(m_vecData.begin(), m_vecData.end(), item);
            if (it != m_vecData.end()) {
                m_vecData.erase(it);
                _ret = true;
            }
            return _ret;
        }

        /// <summary>
        /// Entfernt ein Element aus der Datenstruktur an der durch den Iterator angegebenen Position.
        /// </summary>
        /// <param name="item">Ein Iterator, der auf das zu entfernende Element zeigt.</param>
        iterator remove(const iterator& item) {
            return m_vecData.erase(item);
        }

        /// <summary>
        /// Fügt ein Element am Ende der Liste hinzu oder sortiert es ein, wenn Autosort aktiviert ist.
        /// </summary>
        /// <param name="value">Das einzufügende Element.</param>
        void push_back(const T& value) {
            if (base_type::m_bAutosort) {
                auto it = std::lower_bound(m_vecData.begin(), m_vecData.end(), value, m_funcCompare);
                m_vecData.insert(it, value);
                base_type::m_isSorted = true;
            }
            else {
                m_vecData.push_back(value);
                base_type::m_isSorted = false;
            }
        }

        /// <summary>
        /// Fügt ein Element am Ende der Liste hinzu oder sortiert es ein, abhängig von der Autosort-Einstellung.
        /// </summary>
        /// <param name="value">Das hinzuzufügende Element.</param>
        void push_back(T&& value) {
            if (base_type::m_bAutosort) {
                auto it = std::lower_bound(m_vecData.begin(), m_vecData.end(), value, m_funcCompare);
                m_vecData.insert(it, std::move(value));
                base_type::m_isSorted = true;
            }
            else {
                m_vecData.push_back(std::move(value));
                base_type::m_isSorted = false;
            }
        }
        /// <summary>
        /// Sortiert die Elemente in m_vecData mit der Vergleichsfunktion m_funcCompare.
        /// </summary>
        void sort() {
            std::sort(m_vecData.begin(), m_vecData.end(), m_funcCompare);
            base_type::m_isSorted = true;
        }

        /// <summary>
        /// Löscht alle Elemente aus dem Container.
        /// </summary>
        void clear() {
            m_vecData.clear();
        }

        /// <summary>
        /// Gibt die Anzahl der Elemente im Container zurück.
        /// </summary>
        /// <returns>Die Anzahl der Elemente als Wert vom Typ size_t.</returns>
        size_t size() const {
            return m_vecData.size();
        }

        /// <summary>
        /// Prüft, ob die Datenstruktur leer ist.
        /// </summary>
        /// <returns>Gibt true zurück, wenn die Datenstruktur keine Elemente enthält, andernfalls false.</returns>
        bool empty() const {
            return m_vecData.empty();
        }

        /// <summary>
        /// Gibt das Element am angegebenen Index zurück.
        /// </summary>
        /// <param name="index">Der Index des Elements, das zurückgegeben werden soll.</param>
        /// <returns>Eine konstante Referenz auf das Element am angegebenen Index.</returns>
        const T& operator[](size_t index) const {
            return m_vecData[index];
        }

        /// <summary>
        /// Gibt eine Referenz auf das Element am angegebenen Index im Vektor zurück.
        /// </summary>
        /// <param name="index">Der Index des Elements, das abgerufen werden soll.</param>
        /// <returns>Eine Referenz auf das Element am angegebenen Index.</returns>
        T& operator[](size_t index) {
            return m_vecData[index];
        }
    private:
        /// <summary>
        /// Ein Vektor, der eine Sequenz von Elementen speichert.
        /// </summary>
        std::vector<T> m_vecData;
        /// <summary>
        /// Ein Funktionszeiger zum Vergleichen von Werten.
        /// </summary>
        compare_func_t m_funcCompare;
    };
}