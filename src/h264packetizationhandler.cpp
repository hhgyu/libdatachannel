/*
 * libdatachannel client example
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

#if RTC_ENABLE_MEDIA

#include "h264packetizationhandler.hpp"

namespace rtc {

H264PacketizationHandler::H264PacketizationHandler(shared_ptr<H264RtpPacketizer> packetizer): MediaChainableHandler(packetizer) { }

} // namespace rtc

#endif /* RTC_ENABLE_MEDIA */
