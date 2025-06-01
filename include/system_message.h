/**
* SES Smart Event System
* (c) 2025 Amber-Sophia Schröck
* This file is copyright under the latest version of the EUPL.
* Please see LICENSE file for your rights under this license.
**/
#pragma once

#include "message.h"

namespace ses {

	class SES_API system_message : public message {
	public:
		/// <summary>
		/// Konstruiert ein message-Objekt mit Standardwerten.
		/// </summary>
		system_message(uint32_t system_id, uint8_t prio) 
			: message(prio, 0, true, false), m_systemID(system_id) {}

		virtual bool onMessageProcess(void* sender) = 0;

		virtual bool is_expired(uint64_t now) const { return false; }
		virtual void onMessageExpired(void* sender, uint64_t time) {}
		virtual void onMessageDiscard(void* sender, uint64_t time) {}
		virtual void onMessagePost(void* sender, bool bWasAdd) {}

		uint32_t get_system_msg() const { return m_systemID;  }
	private:
		uint32_t m_systemID;
	};
}