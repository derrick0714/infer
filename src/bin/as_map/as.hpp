#ifndef AS_HPP
#define AS_HPP

class AS {
  public:
    uint32_t firstIP;
    uint32_t lastIP;
    AS(const uint32_t &_firstIP, const uint32_t &_lastIP);
    bool operator!=(const AS &right) const;
    bool operator<(const AS &right) const;
};

AS::AS(const uint32_t &_firstIP, const uint32_t &_lastIP) {
  firstIP = _firstIP;
  lastIP = _lastIP;
}

bool AS::operator!=(const AS &right) const {
  return (firstIP != right.firstIP || lastIP != right.lastIP);
}

bool AS::operator<(const AS &right) const {
  if (firstIP != right.firstIP) {
    return (firstIP < right.firstIP);
  }
  else {
    return (lastIP < right.lastIP);
  }
}

#endif
