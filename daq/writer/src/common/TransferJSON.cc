#include "emu/daq/writer/TransferJSON.h"
#include <sstream>

std::ostream& operator<<( std::ostream& os, const TransferJSON& transferJSON ){
  // See p.4 of https://indico.cern.ch/event/403689/contributions/1848803/attachments/808820/1108418/PDT_mk4.pdf
  os << "{\"data\":["
     << "0,"
     << transferJSON.getNumberOfEvents() << ","
     << "0,"
     << "\"" << transferJSON.getDataFileName() << "\","
     << transferJSON.getDataFileSize() << ","
     << transferJSON.getCheckSum() << ","
     << "0,"
     << "0,"
     << "0,"
     << "\"" << transferJSON.getSymbolicLink() << "\""
     << "]}";
  return os;
}

std::string TransferJSON::getJSONFileName() const{
  std::ostringstream oss;
  oss << "run" << runNumber_ << "_ls" << lsNumber_ << "_stream" << streamName_ << "_" << hostName_ <<  ".jsn";
  return oss.str();
}
