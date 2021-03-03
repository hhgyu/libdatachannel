/**
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RTC_MEDIA_HANDLER_ELEMENT_H
#define RTC_MEDIA_HANDLER_ELEMENT_H

#if RTC_ENABLE_MEDIA

#include "common.hpp"
#include "message.hpp"
#include "rtp.hpp"

namespace rtc {

using ChainedMessagesProduct = std::shared_ptr<std::vector<binary_ptr>>;

RTC_CPP_EXPORT ChainedMessagesProduct make_chained_messages_product();
RTC_CPP_EXPORT ChainedMessagesProduct make_chained_messages_product(message_ptr msg);

/// Ougoing messages
struct RTC_CPP_EXPORT ChainedOutgoingProduct {
	ChainedOutgoingProduct(ChainedMessagesProduct messages = nullptr, message_ptr control = nullptr);
	const ChainedMessagesProduct messages;
	const message_ptr control;
};

/// Incoming messages with response
struct RTC_CPP_EXPORT ChainedIncomingProduct {
	ChainedIncomingProduct(ChainedMessagesProduct incoming = nullptr, ChainedMessagesProduct outgoing = nullptr);
	const ChainedMessagesProduct incoming;
	const ChainedOutgoingProduct outgoing;
};

/// Incoming control messages with response
struct RTC_CPP_EXPORT ChainedIncomingControlProduct {
	ChainedIncomingControlProduct(message_ptr incoming, std::optional<ChainedOutgoingProduct> outgoing = nullopt);
	const message_ptr incoming;
	const std::optional<ChainedOutgoingProduct> outgoing;
};

/// Chainable handler
class RTC_CPP_EXPORT MediaHandlerElement: public std::enable_shared_from_this<MediaHandlerElement> {
	std::shared_ptr<MediaHandlerElement> upstream = nullptr;
	std::shared_ptr<MediaHandlerElement> downstream = nullptr;

	void prepareAndSendResponse(std::optional<ChainedOutgoingProduct> outgoing, std::function<bool (ChainedOutgoingProduct)> send);

	void removeFromChain();
public:
	MediaHandlerElement();

	/// Creates response to incoming message
	/// @param messages Current repsonse
	/// @returns New response
	std::optional<ChainedOutgoingProduct> processOutgoingResponse(ChainedOutgoingProduct messages);

	// Process incoming and ougoing messages
	message_ptr formIncomingControlMessage(message_ptr message, std::function<bool (ChainedOutgoingProduct)> send);
	ChainedMessagesProduct formIncomingBinaryMessage(ChainedMessagesProduct messages, std::function<bool (ChainedOutgoingProduct)> send);
	message_ptr formOutgoingControlMessage(message_ptr message);
	std::optional<ChainedOutgoingProduct> formOutgoingBinaryMessage(ChainedOutgoingProduct product);

	/// Process current control message
	/// @param messages current message
	/// @returns Modified message and response
	virtual ChainedIncomingControlProduct processIncomingControlMessage(message_ptr messages);

	/// Process current control message
	/// @param messages current message
	/// @returns Modified message
	virtual message_ptr processOutgoingControlMessage(message_ptr messages);

	/// Process current binary message
	/// @param messages current message
	/// @returns Modified message and response
	virtual ChainedIncomingProduct processIncomingBinaryMessage(ChainedMessagesProduct messages);

	/// Process current binary message
	/// @param messages current message
	/// @param control current control message
	/// @returns Modified binary message and control message
	virtual ChainedOutgoingProduct processOutgoingBinaryMessage(ChainedMessagesProduct messages, message_ptr control);

	/// Set given element as upstream to this
	/// @param upstream Upstream element
	/// @returns Upstream element
	std::shared_ptr<MediaHandlerElement> chainWith(std::shared_ptr<MediaHandlerElement> upstream);

	/// Remove all downstream elements from chain
	void recursiveRemoveChain();
};

} // namespace rtc

#endif // RTC_ENABLE_MEDIA

#endif // RTC_MEDIA_HANDLER_ELEMENT_H
