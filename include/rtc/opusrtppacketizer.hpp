/*
 * libdatachannel streamer example
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

#ifndef RTC_OPUS_RTP_PACKETIZER_H
#define RTC_OPUS_RTP_PACKETIZER_H

#if RTC_ENABLE_MEDIA

#include "rtppacketizer.hpp"
#include "mediahandlerrootelement.hpp"

namespace rtc {

/// RTP packetizer for opus
class RTC_CPP_EXPORT OpusRtpPacketizer : public RtpPacketizer, public MediaHandlerRootElement {
public:
	/// default clock rate used in opus RTP communication
	static const uint32_t defaultClockRate = 48 * 1000;

	/// Constructs opus packetizer with given RTP configuration.
	/// @note RTP configuration is used in packetization process which may change some configuration
	/// properties such as sequence number.
	/// @param rtpConfig  RTP configuration
	OpusRtpPacketizer(shared_ptr<RtpPacketizationConfig> rtpConfig);

	/// Creates RTP packet for given payload based on `rtpConfig`.
	/// @note This function increase sequence number after packetization.
	/// @param payload RTP payload
	/// @param setMark This needs to be `false` for all RTP packets with opus payload
	binary_ptr packetize(binary_ptr payload, bool setMark) override;

	/// Creates RTP packet for given samples (all samples share same RTP timesamp)
	/// @param messages opus samples
	/// @param control RTCP
	/// @returns RTP packets and unchanged `control`
	ChainedOutgoingProduct processOutgoingBinaryMessage(ChainedMessagesProduct messages, message_ptr control) override;
};

} // namespace rtc

#endif /* RTC_ENABLE_MEDIA */

#endif /* RTC_OPUS_RTP_PACKETIZER_H */
