//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author: Zoltan Bojthe
//

#include "inet/linklayer/acking/AckingMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::ackingmac, AckingMacProtocolDissector);

void AckingMacProtocolDissector::dissect(Packet *packet, ICallback& callback) const
{
    auto header = packet->popHeader<AckingMacHeader>();
    callback.startProtocolDataUnit(&Protocol::ackingmac);
    callback.visitChunk(header, &Protocol::ackingmac);
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(header->getNetworkProtocol());
    callback.dissectPacket(packet, payloadProtocol);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::ackingmac);
}

} // namespace inet
