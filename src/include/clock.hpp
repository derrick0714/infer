class Clock {
  public:
    Clock(std::string, std::string);
    void start();
    void incrementOperations();
    int operations();
    void stop();
  private:
    time_t startTime;
    int numOperations;
    std::string verb;
    std::string noun;
};

Clock::Clock(std::string _verb, std::string _noun) {
  verb = _verb;
  noun = _noun;
}

void Clock::start() {
  numOperations = 0;
  startTime = time(NULL);
}

void Clock::incrementOperations() {
  ++numOperations;
}

int Clock::operations() {
  return numOperations;
}

void Clock::stop() {
  time_t elapsedTime = time(NULL) - startTime;
  std::string timeNoun = "second";
  if (elapsedTime == 0) {
    elapsedTime = 1;
  }
  if (elapsedTime > 1) {
    timeNoun += 's';
  }
  std::cout << verb << ' '  << numOperations << ' ' << noun << " in "
            << elapsedTime << ' '<< timeNoun << " ("
            << numOperations / elapsedTime << ' ' << noun << "/second)"
            << std::endl;
}
