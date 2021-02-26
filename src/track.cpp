/**
 * Copyright (c) 2020 Paul-Louis Ageneau
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

#include "track.hpp"
#include "dtlssrtptransport.hpp"
#include "include.hpp"
#include "logcounter.hpp"

static rtc::LogCounter
    COUNTER_MEDIA_BAD_DIRECTION(plog::warning,
                                "Number of media packets sent in invalid directions");
static rtc::LogCounter COUNTER_QUEUE_FULL(plog::warning,
                                          "Number of media packets dropped due to a full queue");

namespace rtc {

using std::shared_ptr;
using std::weak_ptr;

Track::Track(Description::Media description)
    : mMediaDescription(std::move(description)), mRecvQueue(RECV_QUEUE_LIMIT, message_size_func) {}

string Track::mid() const {
	std::shared_lock lock(mMutex);
	return mMediaDescription.mid();
}

Description::Media Track::description() const {
	std::shared_lock lock(mMutex);
	return mMediaDescription;
}

Description::Direction Track::direction() const {
	std::shared_lock lock(mMutex);
	return mMediaDescription.direction();
}

void Track::setDescription(Description::Media description) {
	std::unique_lock lock(mMutex);
	if (description.mid() != mMediaDescription.mid())
		throw std::logic_error("Media description mid does not match track mid");

	mMediaDescription = std::move(description);
}

void Track::close() {
	mIsClosed = true;

	setRtcpHandler(nullptr);
	resetCallbacks();
}

bool Track::send(message_variant data) {
	if (mIsClosed)
		throw std::runtime_error("Track is closed");

	auto dir = direction();
	if ((dir == Description::Direction::RecvOnly || dir == Description::Direction::Inactive)) {
		COUNTER_MEDIA_BAD_DIRECTION++;
		return false;
	}

	auto message = make_message(std::move(data));

	if (auto handler = getRtcpHandler()) {
		message = handler->outgoing(message);
		if (!message)
			return false;
	}

	return outgoing(std::move(message));
}

bool Track::send(const byte *data, size_t size) { return send(binary(data, data + size)); }

std::optional<message_variant> Track::receive() {
	if (auto next = mRecvQueue.tryPop())
		return to_variant(std::move(**next));

	return nullopt;
}

std::optional<message_variant> Track::peek() {
	if (auto next = mRecvQueue.peek())
		return to_variant(std::move(**next));

	return nullopt;
}

bool Track::isOpen(void) const {
#if RTC_ENABLE_MEDIA
	std::shared_lock lock(mMutex);
	return !mIsClosed && mDtlsSrtpTransport.lock();
#else
	return !mIsClosed;
#endif
}

bool Track::isClosed(void) const { return mIsClosed; }

size_t Track::maxMessageSize() const {
	return 65535 - 12 - 4; // SRTP/UDP
}

size_t Track::availableAmount() const { return mRecvQueue.amount(); }

#if RTC_ENABLE_MEDIA
void Track::open(shared_ptr<DtlsSrtpTransport> transport) {
	{
		std::lock_guard lock(mMutex);
		mDtlsSrtpTransport = transport;
	}

	triggerOpen();
}
#endif

void Track::incoming(message_ptr message) {
	if (!message)
		return;

	auto dir = direction();
	if ((dir == Description::Direction::SendOnly || dir == Description::Direction::Inactive) &&
	    message->type != Message::Control) {
		COUNTER_MEDIA_BAD_DIRECTION++;
		return;
	}

	if (auto handler = getRtcpHandler()) {
		message = handler->incoming(message);
		if (!message)
			return;
	}

	// Tail drop if queue is full
	if (mRecvQueue.full()) {
		COUNTER_QUEUE_FULL++;
		return;
	}

	mRecvQueue.push(message);
	triggerAvailable(mRecvQueue.size());
}

bool Track::outgoing([[maybe_unused]] message_ptr message) {
#if RTC_ENABLfiE_MEDIA
	std::shared_ptr<DtlsSrtpTransport> transport;
	{
		std::shared_lock lock(mMutex);
		transport = mDtlsSrtpTransport.lock();
		if (!transport)
			throw std::runtime_error("Track is closed");

		// Set recommended medium-priority DSCP value
		// See https://tools.ietf.org/html/draft-ietf-tsvwg-rtcweb-qos-18
		if (mMediaDescription.type() == "audio")
			message->dscp = 46; // EF: Expedited Forwarding
		else
			message->dscp = 36; // AF42: Assured Forwarding class 4, medium drop probability
	}

	return transport->sendMedia(message);
#else
	PLOG_WARNING << "Ignoring track send (not compiled with media support)";
	return false;
#endif
}

void Track::setRtcpHandler(std::shared_ptr<MediaHandler> handler) {
	{
		std::unique_lock lock(mMutex);
		mRtcpHandler = handler;
	}

	handler->onOutgoing(std::bind(&Track::outgoing, this, std::placeholders::_1));
}

bool Track::requestKeyframe() {
	if (auto handler = getRtcpHandler())
		return handler->requestKeyframe();

	return false;
}

std::shared_ptr<MediaHandler> Track::getRtcpHandler() {
	std::shared_lock lock(mMutex);
	return mRtcpHandler;
}

} // namespace rtc
