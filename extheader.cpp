#include <iostream>
#include "extheader.h"

/* Little utilities for checking/testing the extended header code. */

std::ostream& operator<<(std::ostream& out, const NEUEVWAV& n) {
    out << "Electrode #\t\t" << n.electrodeID << "\n"
	<< "Location\t\t" << (unsigned int) n.frontEndID << '.' << (unsigned int) n.pin << "\n"
	<< "Neural scale factor\t" << n.neuralScaleFactor << "\n"
	<< "Energy Threshold\t" << n.energyThreshold << "\n"
	<< "Thresholds\t\t (" << n.lowThreshold << "-" << n.highThreshold << ")\n"
	<< "# of units\t\t" << (unsigned int) n.nSorted << "\n"
	<< "bytes per Waveform\t" << (unsigned int) n.bytesPerSample << "\n"
	<< "Stim scale factor\t" << n.stimScaleFactor << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream &out, const SpikeFilter &sf) {
  out << "Electrode " << sf.electrodeID << " spike filter\n:"
      << "\tLowpass Filter: " << sf.LPFilter << "\n"
      << "\tHighpass Filter: " << sf.HPFilter << std::endl;

  return out;
}

std::ostream& operator<<(std::ostream &out, const SpikeHeader &sh) {
  out << "Electrode " << sh.electrodeID << " recording header:\n"
      << "\tLocation: " << (unsigned int) sh.frontEndID << "." << (unsigned int) sh.pin << "\n"
      << "\tEnergy Threshold:" << (unsigned int)  sh.energyThreshold << "\n"
      << "\tThresholds: " << sh.lowThreshold << "and/or" << sh.highThreshold << " uV\n"
      << "\tSorted units: " << (unsigned int) sh.nSorted << "\n"
      << "\tWaveform bit-depth " << (unsigned int) sh.bytesPerSample << "\n"
      << "\tWaveform V/LSB " << sh.scaleFactor << std::endl;

  return out;
}

std::ostream& operator<<(std::ostream &out, const StimHeader &sh) {
  out << "Electrode " << sh.electrodeID << "stimulation header:\n" 
      << "\tLocation" << (unsigned int) sh.frontEndID << "." << (unsigned int) sh.pin << "\n"
      << "\tWaveform bit-depth: " << (unsigned int) sh.bytesPerSample << "\n"
      << "\tWaveform V/LSB: " << sh.scaleFactor << std::endl;
  
  return out;
}
