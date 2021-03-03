/**
 * Copyright (c) 2019-2021 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RTC_IMPL_CHANNEL_H
#define RTC_IMPL_CHANNEL_H

#include "common.hpp"
#include "message.hpp"

#include <atomic>
#include <functional>
#include <variant>

namespace rtc::impl {

struct Channel {
	virtual std::optional<message_variant> receive() = 0;
	virtual std::optional<message_variant> peek() = 0;
	virtual size_t availableAmount() const = 0;

	virtual void triggerOpen();
	virtual void triggerClosed();
	virtual void triggerError(string error);
	virtual void triggerAvailable(size_t count);
	virtual void triggerBufferedAmount(size_t amount);

	virtual void resetCallbacks();

	synchronized_callback<> openCallback;
	synchronized_callback<> closedCallback;
	synchronized_callback<string> errorCallback;
	synchronized_callback<message_variant> messageCallback;
	synchronized_callback<> availableCallback;
	synchronized_callback<> bufferedAmountLowCallback;

	std::atomic<size_t> bufferedAmount = 0;
	std::atomic<size_t> bufferedAmountLowThreshold = 0;
};

} // namespace rtc::impl

#endif
