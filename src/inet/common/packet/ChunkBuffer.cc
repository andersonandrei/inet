//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "inet/common/packet/ByteCountChunk.h"
#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

ChunkBuffer::ChunkBuffer(const char *name) :
    cNamedObject(name)
{
}

ChunkBuffer::ChunkBuffer(const ChunkBuffer& other) :
    cNamedObject(other),
    regions(other.regions)
{
}

void ChunkBuffer::eraseEmptyRegions(std::vector<Region>::iterator begin, std::vector<Region>::iterator end)
{
    // NOTE: begin and end are inclusive
    assert(begin != regions.end());
    assert(end != regions.end());
    regions.erase(std::remove_if(begin, end + 1, [](const Region& region) { return region.data == nullptr; }), end + 1);
}

void ChunkBuffer::sliceRegions(Region& newRegion)
{
    if (!regions.empty()) {
        auto lowerit = std::lower_bound(regions.begin(), regions.end(), newRegion, Region::compareStartOffset);
        if (lowerit != regions.begin())
            lowerit--;
        auto upperit = std::upper_bound(regions.begin(), regions.end(), newRegion, Region::compareEndOffset);
        if (upperit != regions.end())
            upperit++;
        for (auto it = lowerit; it < upperit; it++) {
            auto& oldRegion = *it;
            if (newRegion.getEndOffset() <= oldRegion.getStartOffset() || oldRegion.getEndOffset() <= newRegion.getStartOffset())
                // no intersection
                continue;
            else if (newRegion.getStartOffset() <= oldRegion.getStartOffset() && oldRegion.getEndOffset() <= newRegion.getEndOffset()) {
                // new totally covers old
                oldRegion.data = nullptr;
                regions.erase(it--);
                upperit--;
            }
            else if (oldRegion.getStartOffset() < newRegion.getStartOffset() && newRegion.getEndOffset() < oldRegion.getEndOffset()) {
                // new splits old into two parts
                Region previousRegion(oldRegion.getStartOffset(), oldRegion.data->peek(0, newRegion.getStartOffset() - oldRegion.getStartOffset()));
                Region nextRegion(newRegion.getEndOffset(), oldRegion.data->peek(newRegion.getEndOffset() - oldRegion.getStartOffset(), oldRegion.getEndOffset() - newRegion.getEndOffset()));
                oldRegion.offset = nextRegion.offset;
                oldRegion.data = nextRegion.data;
                regions.insert(it, previousRegion);
                return;
            }
            else if (oldRegion.getEndOffset() <= newRegion.getEndOffset()) {
                // new cuts end of old
                oldRegion.data = oldRegion.data->peek(0, newRegion.getStartOffset() - oldRegion.getStartOffset());
            }
            else if (newRegion.getStartOffset() <= oldRegion.getStartOffset()) {
                // new cuts beginning of old
                oldRegion.data = oldRegion.data->peek(newRegion.getEndOffset() - oldRegion.getStartOffset(), oldRegion.getEndOffset() - newRegion.getEndOffset());
            }
            else
                assert(false);
        }
    }
}

void ChunkBuffer::mergeRegions(Region& previousRegion, Region& nextRegion)
{
    assert(previousRegion.data != nullptr);
    assert(nextRegion.data != nullptr);
    if (previousRegion.getEndOffset() == nextRegion.getStartOffset()) {
        // consecutive regions
        if (previousRegion.data->canInsertAtEnd(nextRegion.data)) {
            // merge into previous
            if (previousRegion.data.use_count() <= 2)
                previousRegion.data->markMutableIfExclusivelyOwned();
            else
                previousRegion.data = previousRegion.data->dupShared();
            previousRegion.data->insertAtEnd(nextRegion.data);
            previousRegion.data = previousRegion.data->peek(0, previousRegion.data->getChunkLength());
            previousRegion.data->markImmutable();
            nextRegion.data = nullptr;
        }
        else if (nextRegion.data->canInsertAtBeginning(previousRegion.data)) {
            // merge into next
            if (nextRegion.data.use_count() <= 2)
                nextRegion.data->markMutableIfExclusivelyOwned();
            else
                nextRegion.data = nextRegion.data->dupShared();
            nextRegion.data->insertAtBeginning(previousRegion.data);
            nextRegion.data = nextRegion.data->peek(0, nextRegion.data->getChunkLength());
            nextRegion.data->markImmutable();
            nextRegion.offset = previousRegion.offset;
            previousRegion.data = nullptr;
        }
        else {
            // merge as a sequence
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertAtEnd(previousRegion.data);
            sequenceChunk->insertAtEnd(nextRegion.data);
            sequenceChunk->markImmutable();
            previousRegion.data = sequenceChunk;
            nextRegion.data = nullptr;
        }
    }
}

void ChunkBuffer::replace(int64_t offset, const std::shared_ptr<Chunk>& chunk)
{
    assert(offset >= 0);
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    Region newRegion(offset, chunk);
    sliceRegions(newRegion);
    if (regions.empty())
        regions.push_back(newRegion);
    else if (regions.back().getEndOffset() <= offset) {
        regions.push_back(newRegion);
        mergeRegions(regions[regions.size() - 2], regions[regions.size() - 1]);
        eraseEmptyRegions(regions.end() - 2, regions.end() - 1);
    }
    else if (offset + chunk->getChunkLength() <= regions.front().getStartOffset()) {
        regions.insert(regions.begin(), newRegion);
        mergeRegions(regions[0], regions[1]);
        eraseEmptyRegions(regions.begin(), regions.begin() + 1);
    }
    else {
        auto it = std::upper_bound(regions.begin(), regions.end(), newRegion, Region::compareStartOffset);
        it = regions.insert(it, newRegion);
        auto& previousRegion = *(it - 1);
        auto& region = *it;
        auto& nextRegion = *(it + 1);
        if (it != regions.begin())
            mergeRegions(previousRegion, region);
        if (it + 1 != regions.end())
            mergeRegions(region.data != nullptr ? region : previousRegion, nextRegion);
        eraseEmptyRegions(it != regions.begin() ? it - 1 : it, it + 1 != regions.end() ? it + 1 : it);
    }
}

void ChunkBuffer::clear(int64_t offset, int64_t length)
{
    assert(offset >= 0);
    assert(length >= 0);
    for (auto it = regions.begin(); it != regions.end(); it++) {
        auto region = *it;
        if (region.offset == offset && region.data->getChunkLength() == length) {
            regions.erase(it);
            return;
        }
    }
    auto data = std::make_shared<ByteCountChunk>(length);
    Region clearedRegion(offset, data);
    sliceRegions(clearedRegion);
}

std::string ChunkBuffer::str() const
{
    std::ostringstream os;
    os << "ChunkBuffer, regions = {";
    bool first = true;
    for (auto& region : regions) {
        if (!first)
            os << " | ";
        else
            first = false;
        os << "offset = " << region.offset << ", chunk = ";
        if (region.data == nullptr)
            os << "<nullptr>";
        else
            os << region.data;
    }
    os << "}";
    return os.str();
}

} // namespace