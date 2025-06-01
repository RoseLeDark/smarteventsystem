/**
* SES Smart Event System
* (c) 2025 Amber-Sophia Schröck
* This file is copyright under the latest version of the EUPL.
* Please see LICENSE file for your rights under this license.
**/
#pragma once

#include <functional>
#include <iostream>

namespace ses {
    /// <summary>
    /// Abstrakte Basisklasse für eine sortierte Containerstruktur mit optionaler automatischer Sortierung.
    /// </summary>
    /// <typeparam name="T">Der Typ des zugrunde liegenden Containers.</typeparam>
    template<class T>
    class sorted {
    public:
        /// <summary>
        /// Definiert einen Funktionsalias für einen Vergleichsoperator zwischen zwei Objekten desselben Typs.
        /// </summary>
        using compare_func_t = std::function<bool(const T, const T)>;

        /// <summary>
        /// Konstruiert ein 'sorted'-Objekt mit der angegebenen automatischen Sortieroption.
        /// </summary>
        /// <param name="autoSorted">Legt fest, ob die automatische Sortierung aktiviert ist.</param>
        explicit sorted(bool autoSorted ) : m_bAutosort(autoSorted), m_isSorted(false) {}

        /// <summary>
        /// Setzt die Vergleichsfunktion für die Verarbeitung.
        /// </summary>
        /// <param name="func">Die Funktionszeiger- oder Funktionsobjekt, das für Vergleiche verwendet werden soll.</param>
        virtual void set_handle(compare_func_t func) = 0;

        /// <summary>
        /// Sortiert die Elemente in einer abgeleiteten Klasse.
        /// </summary>
        virtual void sort() = 0;
     

		/// <summary>
		/// Gibt an, ob die Elemente sortiert sind.
		/// </summary>
		/// <returns>true, wenn die Elemente sortiert sind; andernfalls false.</returns>
		bool is_sorted() const { return m_isSorted; }
		/// <summary>
		/// Gibt an, ob die automatische Sortierung aktiviert ist.
		/// </summary>
		/// <returns>true, wenn die automatische Sortierung aktiviert ist, andernfalls false.</returns>
		bool is_autosort() const { return m_bAutosort; }

        /// <summary>
        /// Setzt die automatische Sortierung.
        /// </summary>
        /// <param name="autosort">Legt fest, ob die automatische Sortierung aktiviert werden soll.</param>
        void set_autosort(bool autosort) {
            m_bAutosort = autosort;
		}
    protected:
        /// <summary>
        /// Gibt an, ob die automatische Sortierung aktiviert ist.
        /// </summary>
        bool m_bAutosort;
        /// <summary>
        /// Gibt an, ob die Elemente sortiert sind.
        /// </summary>
        bool m_isSorted;
    };

}
 