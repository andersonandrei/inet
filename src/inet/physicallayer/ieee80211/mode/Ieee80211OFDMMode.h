//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211OFDMMODE_H
#define __INET_IEEE80211OFDMMODE_H

#include "inet/physicallayer/ieee80211/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMCode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMModeBase
{
  protected:
    Hz channelSpacing;
    Hz bandwidth;

  protected:
    bps computeGrossBitrate(const Ieee80211OFDMModulation *modulation) const;
    bps computeNetBitrate(bps grossBitrate, const Ieee80211OFDMCode *code) const;

  public:
    Ieee80211OFDMModeBase(Hz channelSpacing, Hz bandwidth);

    Hz getSubcarrierFrequencySpacing() const { return channelSpacing / 64; }
    simtime_t getFFTTransformPeriod() const { return simtime_t(1 / getSubcarrierFrequencySpacing().get()); }
    simtime_t getGIDuration() const { return getFFTTransformPeriod() / 4; }
    simtime_t getSymbolInterval() const { return getGIDuration() + getFFTTransformPeriod(); }

    Hz getChannelSpacing() const { return channelSpacing; }
    Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211OFDMPreambleMode : public Ieee80211OFDMModeBase
{
  public:
    Ieee80211OFDMPreambleMode(Hz channelSpacing, Hz bandwidth);

    simtime_t getTrainingSymbolGIDuration() const { return getFFTTransformPeriod() / 2; }
    simtime_t getShortTrainingSequenceDuration() const { return 10 * getFFTTransformPeriod() / 4; }
    simtime_t getLongTrainingSequenceDuration() const { return getTrainingSymbolGIDuration() + 2 * getFFTTransformPeriod(); }
    simtime_t getDuration() const { return getShortTrainingSequenceDuration() + getLongTrainingSequenceDuration(); }
};

class INET_API  Ieee80211OFDMSignalMode: public Ieee80211OFDMModeBase
{
  protected:
    const Ieee80211OFDMCode *code;
    const Ieee80211OFDMModulation *modulation;
    bps netBitrate;
    bps grossBitrate;
    unsigned int rate;

  public:
    Ieee80211OFDMSignalMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth, unsigned int rate);

    const Ieee80211OFDMCode* getCode() const { return code; }
    const Ieee80211OFDMModulation* getModulation() const { return modulation; }
    simtime_t getDuration() const { return getSymbolInterval(); }

    bps getGrossBitrate() const { return grossBitrate; }
    bps getNetBitrate() const { return netBitrate; }
    unsigned int getRate() const { return rate; }

};

class INET_API Ieee80211OFDMDataMode : public Ieee80211OFDMModeBase
{
  protected:
    const Ieee80211OFDMCode *code;
    const Ieee80211OFDMModulation *modulation;
    bps netBitrate;
    bps grossBitrate;

  public:
    Ieee80211OFDMDataMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth);

    const Ieee80211OFDMCode* getCode() const { return code; }
    const Ieee80211OFDMModulation* getModulation() const { return modulation; }
    const bps getGrossBitrate() const { return grossBitrate; }
    const bps getNetBitrate() const { return netBitrate; }
};

class INET_API Ieee80211OFDMMode : public Ieee80211OFDMModeBase
{
  protected:
    const Ieee80211OFDMPreambleMode *preambleMode;
    const Ieee80211OFDMSignalMode *signalMode;
    const Ieee80211OFDMDataMode *dataMode;

  public:
    Ieee80211OFDMMode(const Ieee80211OFDMPreambleMode *preambleMode, const Ieee80211OFDMSignalMode *signalMode, const Ieee80211OFDMDataMode *dataMode, Hz channelSpacing, Hz bandwidth);

    int getNumberOfDataSubcarriers() const { return 48; }
    int getNumberOfPilotSubcarriers() const { return 4; }
    int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }

    const Ieee80211OFDMDataMode* getDataMode() const { return dataMode; }
    const Ieee80211OFDMSignalMode* getSignalMode() const { return signalMode; }
    const Ieee80211OFDMPreambleMode* getPreambleMode() const { return preambleMode; }

    // TODO: Table 18-17—OFDM PHY characteristics
};

class INET_API Ieee80211OFDMCompliantModes
{
  public:
    // Preamble modes: 18.3.3 PLCS preamble (SYNC).
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS5MHz;
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS10MHz;
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS20MHz;

    // Signal modes: Table 18-6—Contents of the SIGNAL field
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate3;

    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate3;

    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate3;

    // Data modes: Table 18-4—Modulation-dependent parameters
    static const Ieee80211OFDMDataMode ofdmDataMode1_5Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode2_25Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode3MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode3MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode4_5MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode4_5MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode13_5Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode18MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode18MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode24MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode24MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode27Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode36Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode48Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode54Mbps;

    // Modes
    static const Ieee80211OFDMMode ofdmMode1_5Mbps;
    static const Ieee80211OFDMMode ofdmMode2_25Mbps;
    static const Ieee80211OFDMMode ofdmMode3MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode3MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode4_5MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode4_5MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode13_5Mbps;
    static const Ieee80211OFDMMode ofdmMode18MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode18MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode24MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode24MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode27Mbps;
    static const Ieee80211OFDMMode ofdmMode36Mbps;
    static const Ieee80211OFDMMode ofdmMode48Mbps;
    static const Ieee80211OFDMMode ofdmMode54Mbps;

  public:
    static const Ieee80211OFDMMode& getCompliantMode(unsigned int signalRateField, Hz channelSpacing);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211OFDMMODE_H
