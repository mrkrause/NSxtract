#ifndef __NEVFile.h__
#define __NEVFile.h__

#include <cstdint>
#include <ifstream>
#include <stdexcept>
#include <string>


class NEVFile {
 public:
  NEVFile(std::string filename);


 protected:
  std::ifstream file;

  unsigned char majorVersion;
  unsigned char minorVersion;

  std::uint16_t flags;

  std::uint32_t offset;
  std::uint32_t packetSize;

  std::uint32_t tsResolution;
  std::uint32_t sampResolution;

  SystemTime time;

  char label[16];
  char comment[256];

  std::uint32_t nHeaders;
}
