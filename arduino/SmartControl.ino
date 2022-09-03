#include <EEPROM.h>
#include <RTClib.h>

const unsigned int firstPort = 2;
const unsigned int lastPort = 13;
const unsigned int uploadDelayMs = 100;
const unsigned int syncDelayMs = 10;
const unsigned int syncIntervalMs = 5 * 60 * 1000;
const unsigned int EVENT_WIDTH_MINS = 2;

typedef struct DayTime {
  int hours : 5;
  int minutes : 6;
};

typedef struct Timeout {
  unsigned int port : 5;
  bool state = false;
  unsigned int unixTime;
  DayTime dayTime;
};

typedef struct Weekly {
  unsigned int port : 5;
  bool state = false;
  unsigned int days : 7;
  DayTime dayTime;
  bool loop = true;
  unsigned int count = 0;
};

typedef struct Scheduled {
  unsigned int port : 5;
  bool state = false;
  unsigned int unixTime;
  DayTime dayTime;
  bool loop = false;
  unsigned int count = 0;
  unsigned int intervalDays = 1;
};

typedef struct DirectEvent {
  DayTime dayTime;
  unsigned int unixTime = 0;
};

RTC_DS1307 rtc;
int timeoutCount = 0;
Timeout *timeouts;
int weeklyCount = 0;
Weekly *weeklies;
int scheduledCount = 0;
Scheduled *scheduleds;
DirectEvent directEvents[lastPort - firstPort + 1];
class OptimizedProvider { /*TODO: more optimization to reduce call backs to rtc.*/
public:
  DayTime getDayTime() {
    DateTime now = rtc.now();
    DayTime dayTime;
    dayTime.hours = now.hour();
    dayTime.minutes = now.minute();
    return dayTime;
  }
  unsigned int getUnixTime() {
    DateTime now = rtc.now();
    return now.unixtime();
  }
  unsigned int getDay() {
    DateTime now = rtc.now();
    return now.day();
  }
};

OptimizedProvider time;


void setup() {
  //Setup hardware
  setUpRTC();
  for (unsigned int port = firstPort; port <= lastPort; port++) {
    pinMode(port, OUTPUT);
  }

  //Now, load data from eeprom
  fetchData();
  /* ... */
}

void fetchData() {
  int offset = 0;
  timeoutCount = EEPROM.read(0);
  offset += 4;
  weeklyCount = EEPROM.read(offset);
  offset += 4;
  scheduledCount = EEPROM.read(offset);
  offset += 8;  //One more for future usage

  if (timeoutCount < 0 || weeklyCount < 0 || scheduledCount < 0) { /*TODO: include more reliable eeprom intefgrity check.*/
    saveData(13);
    return;
  }

  if (timeoutCount) {
    timeouts = realloc(timeouts, timeoutCount);  //Memory leak is not our concern
    EEPROM.get(offset, timeouts);
  }
  offset += sizeof(Timeout) * timeoutCount;
  if (weeklyCount) {
    weeklies = realloc(weeklies, weeklyCount);
    EEPROM.get(offset, weeklies);
  }
  offset += sizeof(Weekly) * weeklyCount;
  if (scheduledCount) {
    scheduleds = realloc(scheduleds, scheduledCount);
    EEPROM.get(offset, scheduleds);
  }
  offset += sizeof(Scheduled) * scheduledCount;
  EEPROM.get(offset, directEvents);
}

void saveData(int what) {
  int offset = 4 * 4;
  if (what & 1) {
    EEPROM.write(0, timeoutCount);
    EEPROM.put(offset, timeouts);
  }
  offset += timeoutCount * sizeof(Timeout);
  if (what & 2) {
    EEPROM.write(4, weeklyCount);
    EEPROM.put(offset, weeklies);
  }
  offset += weeklyCount * sizeof(Weekly);
  if (what & 4) {
    EEPROM.write(8, scheduledCount);
    EEPROM.put(offset, scheduleds);
  }
  offset += scheduledCount * sizeof(Scheduled);
  if (what & 8) {
    EEPROM.put(offset, directEvents);
  }
}

void setUpRTC() {
#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  time = OptimizedProvider();
}

void loop() {
  /* ... */
}
/**All excution of events should be first computed in virtual memory before applying to hardware. The reason behind this is that is allows prevalidation. For exapmle comparision between out put of events, and any hot user intervention.*/
bool *checkEvents() {
  /* ... */
}
bool testTimeOut(Timeout timeout, DayTime daytime, unsigned int unixtime) {
  int dif = (unixtime - timeout.unixTime) * 1440 + (daytime.hours - timeout.dayTime.hours) * 60 + (daytime.minutes - timeout.dayTime.minutes);
  if (dif > 0 && dif < EVENT_WIDTH_MINS) return true;
  else return false;
}
bool testWeekly(Weekly weekly, DayTime daytime, unsigned int day) {
  if (weekly.days & 1 << day) {
    int dif = (daytime.hours - weekly.dayTime.hours) * 60 + (daytime.minutes - weekly.dayTime.minutes);
    if (dif > 0 && dif < EVENT_WIDTH_MINS) return true;
    else return false;
  } else {
    int dayBefore = (day <= 0) ? 6 : day - 1;
    if (weekly.days & 1 << dayBefore) {
      int dif = 1440 + (daytime.hours - weekly.dayTime.hours) * 60 + (daytime.minutes - weekly.dayTime.minutes);
      if (dif > 0 && dif < EVENT_WIDTH_MINS) return true;
      else return false;
    }
  }
  return false;
}
bool testScheduled(Scheduled scheduled, DayTime daytime, unsigned int unixtime) {
  int dif = (unixtime - scheduled.unixTime) * 1440 + (daytime.hours - scheduled.dayTime.hours) * 60 + (daytime.minutes - scheduled.dayTime.minutes);

  if (dif > 0) {
    if (dif < EVENT_WIDTH_MINS) return true;
    unsigned int daysGap = (unsigned int)(dif / 1440);

    if (daysGap > scheduled.intervalDays * scheduledCount) return -1;
    else if (daysGap % scheduled.intervalDays == 0 && dif % 1440 < EVENT_WIDTH_MINS) return 1;
    else return 0;
  }
  return 0;
}

void excute() {
  /* ... */
}

void handleConnection() {
  /* More code... */
}
/* More and more code... */