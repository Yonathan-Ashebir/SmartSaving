#include <LinkedList.h>
#include <util/atomic.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

//specific for current hardware.
const byte PORTS_COUNT = 14;
const byte PORTS[PORTS_COUNT] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3, A6 };
const byte UPLOAD_DELAY_MS = 100;
const byte SYNC_DELAY_MS = 10;
const unsigned int SYNC_INTERVALS_MS = 5 * 60;
const byte EVENT_WIDTH_MINS = 2;
const unsigned int CUSTOM_TO_UNIX = DateTime(2020, 0, 1, 0, 0, 0).unixtime();
const byte BLUETOOTH_KEY_PIN = 4;
const byte BLUETOOTH_TX_PIN = 3;
const byte BLUETOOTH_RX_PIN = 2;
const byte RTC_SDA_PIN = A4;
const byte RTC_SCL_PIN = A5;
const unsigned REFRESH_INTERVAL_MS = 20000;

struct DayTime {
  DayTime(const unsigned int hours, const unsigned int mins) {
    this->hours = hours;
    this->minutes = mins;
  }
  byte hours : 5;
  byte minutes : 6;
};

struct Flags {
  long dirty:32;
};

struct Timeout {  //info: size = 4 byte
private:
  unsigned int customTime : 16;
  byte hours : 5;
  byte minutes : 6;
  byte port : 4;
  bool state : 1;
public:
  unsigned int getUnixtime() {
    return this->customTime + CUSTOM_TO_UNIX;
  }
  DayTime getDayTime() {
    return DayTime(this->hours, this->minutes);
  }
  unsigned int getPort() {
    return this->port;
  }
  bool getState() {
    return this->state;
  }
  void setUnixtime(int unixtime) {
    this->customTime = unixtime - CUSTOM_TO_UNIX;
  }
  void setDayTime(DayTime dayTime) {
    hours = dayTime.hours;
    minutes = dayTime.minutes;
  }
  void setPort(int port) {
    this->port = port;
  }
  void setState(bool state) {
    this->state = state;
  }
};

struct Weekly {  //info: size = 4 byte
private:
  byte days = 7;
  byte hours : 5;
  byte port : 4;
  byte minutes : 6;
  byte state : 1;
public:
  unsigned int getDays() {
    return this->days;
  }
  bool getDay(int ind) {
    return (1 << ind) & (0xff & this->days);
  }

  DayTime getDayTime() {
    return DayTime(hours, minutes);
  }
  unsigned int getPort() {
    return port;
  }

  bool getState() {
    return state;
  }
  void setDays(unsigned int days) {
    this->days = days;
  }
  void setDay(unsigned int ind, bool val) {
    if (val) {
      this->days |= (1 << ind);
    } else {
      this->days &= ~(1 << ind);
    }
  }

  void setDayTime(DayTime dayTime) {
    hours = dayTime.hours;
    minutes = dayTime.minutes;
  }
  void setPort(unsigned int port) {
    this->port = port;
  }
  void setState(bool state) {
    if (state) {
      this->state = 1;
    } else {
      this->state = 0;
    }
  }
};

struct Scheduled {  //info: size = 6 bytes
private:
  byte customTime1;
  byte customTime2;
  byte port : 4;
  bool state : 1;
  byte hours : 5;
  byte minutes : 6;
  byte count = 0;  //255 means loop
  byte intervalDays = 1;
public:
  unsigned int getUnixtime() {
    return CUSTOM_TO_UNIX + (customTime1 | (customTime2 << 8));
  }
  DayTime getDayTime() {
    return DayTime(this->hours, this->minutes);
  }
  unsigned int getPort() {
    return this->port;
  }
  bool getState() {
    return this->state;
  }
  unsigned int getCount() {
    return this->count;
  }
  unsigned int getIntervalDays() {
    return this->intervalDays;
  }

  void setUnixtime(unsigned int unixtime) {
    unsigned int customTime = unixtime - CUSTOM_TO_UNIX;
    this->customTime1 = 0xff & customTime;
    this->customTime2 = 0xff00 & customTime;
  }
  void setDayTime(DayTime dayTime) {
    this->hours = dayTime.hours;
    minutes = dayTime.minutes;
  }
  void setPort(unsigned int port) {
    this->port = port;
  }
  void setState(bool state) {
    if (state)
      this->state = 1;
    else this->state = 0;
  }
  void setCount(unsigned int count) {
    this->count = count;
  }
  void setIntervalDays(unsigned int intervalDays) {
    this->intervalDays = intervalDays;
  }
};

struct Event {  //info size = 4 bytes
private:
  unsigned int customTime : 16;
  byte hours : 5;
  byte minutes : 6;
  byte state : 1;
public:
  DayTime getDayTime() {
    return DayTime(hours, minutes);
  }
  bool getState() {
    return state;
  }
  unsigned int getUnixtime() {
    return customTime + CUSTOM_TO_UNIX;
  }

  void setDayTime(DayTime dayTime) {
    hours = dayTime.hours;
    minutes = dayTime.minutes;
  }
  void setUnixtime(unsigned int unixtime) {
    customTime = unixtime - CUSTOM_TO_UNIX;
  }
  void setState(bool state) {
    if (state) {
      this->state = 1;
    } else {
      this->state = 0;
    }
  }
};

struct DiffValue {
  int value;
  bool done;
  DiffValue(int value, bool done) {
    this->value = value;
    this->done = done;
  }
};

RTC_DS1307 rtc;

LinkedList<Timeout> timeouts;
LinkedList<Weekly> weeklies;
LinkedList<Scheduled> schedules;

Event events[PORTS_COUNT];
Flags flags;
unsigned int lastUpdateTime = 0;
SoftwareSerial bluetooth(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);  //receive pin - 2, transmitting pin -3

class OptimizedProvider { /*TODO: more optimization to reduce call backs to rtc.*/
public:
  DayTime getDayTime() {
    DateTime now = rtc.now();
    DayTime dayTime(now.hour(), now.minute());
    return dayTime;
  }
  unsigned int getUnixtime() {
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
#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif
  //Setup hardware
  for (int ind = 0; ind < PORTS_COUNT; ind++) {
    pinMode(PORTS[ind], OUTPUT);
  }
  setUpRTC();
  setupBluetooth();

  //Now, load data from eeprom
  fetchData();
  //update state/events
  updateState();
  //excute them.
  excute();
}

void loop() {
  unsigned int currentTime = millis();
  handleConnection();
  if (!lastUpdateTime && currentTime - lastUpdateTime > REFRESH_INTERVAL_MS) {
    lastUpdateTime = currentTime;
    updateState();
    excute();
  }
}

void setUpRTC() {
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

void setupBluetooth() {
  delay(100);
  pinMode(BLUETOOTH_KEY_PIN, OUTPUT);
  bluetooth.begin(9600);
  // digitalWrite(4, HIGH);
}

void fetchData() {
  EEPROM.get(12, flags);
  if (flags.dirty) {
    fixData();
    return;
  }
  unsigned int offset = 0;
  unsigned int timeoutsCount = EEPROM.get(offset, timeoutsCount);
  offset += 4;
  unsigned int weekliesCount = EEPROM.get(offset, weekliesCount);
  offset += 4;
  unsigned int scheduledsCount = EEPROM.get(offset, scheduledsCount);
  offset += 8;  //One more for future usage


  if (timeoutsCount) {
    byte size = sizeof(Timeout);
    for (unsigned int ind = 0; ind < timeoutsCount; ind++) {
      Timeout timeout;
      EEPROM.get(offset, timeout);
      timeouts.add(timeout);
      offset += size;
    }
  }

  if (weekliesCount) {
    byte size = sizeof(Weekly);
    for (unsigned int ind = 0; ind < weekliesCount; ind++) {
      Weekly weekly;
      EEPROM.get(offset, weekly);
      weeklies.add(weekly);
      offset += size;
    }
  }

  if (scheduledsCount) {
    byte size = sizeof(Scheduled);
    for (unsigned int ind = 0; ind < weekliesCount; ind++) {
      Scheduled scheduled;
      EEPROM.get(offset, scheduled);
      schedules.add(scheduled);
      offset += size;
    }
  }

  EEPROM.get(offset, events);
}

void saveData(int what) {  //todo: dirty flag with in the extra space
  Flags f;
  memcpy(&f, &flags, 4);
  f.dirty = true;
  EEPROM.put(12, f);
  int offset = 4 * 4;
  if (what & 1) {
    EEPROM.put(0, timeouts.size());
    byte size = sizeof(Timeout);
    for (int ind = 0; ind < timeouts.size(); ind++) {
      EEPROM.put(offset, timeouts.get(ind));
      offset += size;
    }
  }

  if (what & 2) {
    EEPROM.put(4, weeklies.size());
    byte size = sizeof(Weekly);
    for (int ind = 0; ind < weeklies.size(); ind++) {
      EEPROM.put(offset, weeklies.get(ind));
      offset += size;
    }
  }

  if (what & 4) {
    EEPROM.put(8, schedules.size());
    byte size = sizeof(Scheduled);
    for (int ind = 0; ind < schedules.size(); ind++) {
      EEPROM.put(offset, schedules.get(ind));
      offset += size;
    }
  }

  if (what & 8) {
    EEPROM.put(offset, events);
  }
  f.dirty = false;
  EEPROM.put(12, f);
}

void updateState() {
  DayTime dayTime = time.getDayTime();
  unsigned int unixtime = time.getUnixtime();
  DateTime dateTime = rtc.now();

  for (int ind = 0; ind < timeouts.size(); ind++) {
    Timeout timeout = timeouts.get(ind);
    Event event = events[timeout.getPort()];

    signed int timeoutDiff = diffTimeOut(timeout, dayTime, unixtime);
    if (timeoutDiff >= 0) {
      signed int eventDiff = diffEvent(event, event.getDayTime(), event.getUnixtime());
      if (eventDiff == -2) {
        event.setUnixtime(timeout.getUnixtime());
        event.setDayTime(timeout.getDayTime());
        event.setState(timeout.getState());
      } else if (eventDiff > timeoutDiff) {
        event.setUnixtime(timeout.getUnixtime());
        event.setDayTime(timeout.getDayTime());
        event.setState(timeout.getState());
      }
      timeouts.remove(ind);
      ind--;
    }
  }

  for (int ind = 0; ind < weeklies.size(); ind++) {
    Weekly weekly = weeklies.get(ind);
    Event event = events[weekly.getPort()];

    signed int weeklyDiff = diffWeekly(weekly, dayTime, unixtime);
    if (weeklyDiff == -2) {
      weeklies.remove(ind);
      ind--;
      log("Removed invalid weekly", 1);
      continue;
    }

    if (weeklyDiff > 0) {
      signed int eventDiff = diffEvent(event, event.getDayTime(), event.getUnixtime());
      DateTime newEventTime = dateTime - TimeSpan(weeklyDiff * 60);
      if (eventDiff == -2) {
        event.setUnixtime(newEventTime.unixtime());
        event.setDayTime(weekly.getDayTime());
        event.setState(weekly.getState());
      } else if (eventDiff > weeklyDiff) {
        event.setUnixtime(newEventTime.unixtime());
        event.setDayTime(weekly.getDayTime());
        event.setState(weekly.getState());
      }
    }
  }

  for (int ind = 0; ind < schedules.size(); ind++) {
    Scheduled scheduled = schedules.get(ind);
    Event event = events[scheduled.getPort()];

    DiffValue diffValue = diffScheduled(scheduled, dayTime, unixtime);
    signed int scheduledDiff = diffValue.value;
    if (scheduledDiff == -1) continue;
    if (scheduledDiff > 0) {
      signed int eventDiff = diffEvent(event, event.getDayTime(), event.getUnixtime());
      DateTime newEventTime = dateTime - TimeSpan(eventDiff * 60);
      if (eventDiff == -2) {
        event.setUnixtime(newEventTime.unixtime());
        event.setDayTime(scheduled.getDayTime());
        event.setState(scheduled.getState());
      } else if (eventDiff > scheduledDiff) {
        event.setUnixtime(newEventTime.unixtime());
        event.setDayTime(scheduled.getDayTime());
        event.setState(scheduled.getState());
      }
      if (diffValue.done) {
        schedules.remove(ind);
        ind--;
      }
    }
  }
}

void excute() {
  for (int ind = 0; ind < PORTS_COUNT; ind++) {
    Event event = events[ind];
    digitalWrite(PORTS[ind], event.getState());
  }
}


void handleConnection() {
  bool changed = false;
start:
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (String("begin").equals(line)) {
    bluetooth.println("listening");
    bluetooth.setTimeout(3000);
    line = bluetooth.readStringUntil('\n');
    line.trim();
    bluetooth.setTimeout(1000);
    switch (line.toInt()) {
      case 0:
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          if (line.equals("timeout")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Timeout timeout;
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) {
                DateTime datetime(entry.toInt());
                timeout.setUnixtime(datetime.unixtime());
                timeout.setDayTime(DayTime(datetime.hour(), datetime.minute()));
              } else if (count == 1) timeout.setPort(entry.toInt());
              else if (count == 2) timeout.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              timeouts.add(timeout);
              bluetooth.println("success");
              break;
            }

          } else if (line.equals("weekly")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Weekly weekly;
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) weekly.setDays(entry.toInt());
              else if (count == 1) {
                unsigned int i = entry.indexOf(':');
                weekly.setDayTime(DayTime(entry.substring(0, i).toInt(), entry.substring(i).toInt()));
              } else if (count == 2) weekly.setPort(entry.toInt());
              else if (count == 3) weekly.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              weeklies.add(weekly);
              bluetooth.println("success");
              break;
            }

          } else if (line.equals("schedule")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Scheduled schedule;
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) {
                DateTime dateTime(entry.toInt());
                schedule.setUnixtime(dateTime.unixtime());
                schedule.setDayTime(DayTime(dateTime.hour(), dateTime.minute()));
              } else if (count == 1) schedule.setCount(entry.toInt());
              else if (count == 2) schedule.setIntervalDays(entry.toInt());
              else if (count == 3) schedule.setPort(entry.toInt());
              else if (count == 4) schedule.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              schedules.add(schedule);
              bluetooth.println("success");
              break;
            }
          }
          bluetooth.println("fail");
          break;
        }
      case 1:
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          String ind = bluetooth.readStringUntil('\n');
          line.trim();
          if (line.equals("timeout")) {
            timeouts.remove(ind.toInt());
          } else if (line.equals("weekly")) {
            weeklies.remove(ind.toInt());
          } else if (line.equals("schedules")) {
            schedules.remove(ind.toInt());
          } else {
            bluetooth.println("failed");
            break;
          }
          bluetooth.println("success");
          break;
        }
      case 2:
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          if (line.equals("timeout")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Timeout timeout = timeouts.get(line.toInt());
            line = bluetooth.readStringUntil('\n');
            line.trim();
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) {
                DateTime datetime(entry.toInt());
                timeout.setUnixtime(datetime.unixtime());
                timeout.setDayTime(DayTime(datetime.hour(), datetime.minute()));
              } else if (count == 1) timeout.setPort(entry.toInt());
              else if (count == 2) timeout.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              bluetooth.println("success");
              break;
            }

          } else if (line.equals("weekly")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Weekly weekly = weeklies.get(line.toInt());
            line = bluetooth.readStringUntil('\n');
            line.trim();
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) weekly.setDays(entry.toInt());
              else if (count == 1) {
                unsigned int i = entry.indexOf(':');
                weekly.setDayTime(DayTime(entry.substring(0, i).toInt(), entry.substring(i).toInt()));
              } else if (count == 2) weekly.setPort(entry.toInt());
              else if (count == 3) weekly.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              bluetooth.println("success");
              break;
            }

          } else if (line.equals("schedule")) {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            signed int ind = 0;
            unsigned int count = 0;
            Scheduled schedule = schedules.get(line.toInt());
            line = bluetooth.readStringUntil('\n');
            line.trim();
            while (ind != -1) {
              unsigned int nextInd = line.indexOf(';', ind);
              String entry = line.substring(ind, nextInd);
              if (count == 0) {
                DateTime dateTime(entry.toInt());
                schedule.setUnixtime(dateTime.unixtime());
                schedule.setDayTime(DayTime(dateTime.hour(), dateTime.minute()));
              } else if (count == 1) schedule.setCount(entry.toInt());
              else if (count == 2) schedule.setIntervalDays(entry.toInt());
              else if (count == 3) schedule.setPort(entry.toInt());
              else if (count == 4) schedule.setState(entry.toInt());
              count++;
              ind = nextInd;
            }
            line = bluetooth.readStringUntil('\n');
            line.trim();
            if (line.equals("commit") && count == 3) {
              schedules.add(schedule);
              bluetooth.println("success");
              break;
            }
          }
          bluetooth.println("fail");
          break;
        }
      case 3:  //connection
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          if (line == "get") {
            bluetooth.println((String(getBluetoothName()) + ";" + getBluetoothPassword()).c_str());
          } else if (line == "set") {
            line = bluetooth.readStringUntil('\n');
            line.trim();
            unsigned int i = line.indexOf(';');
            setBluetoothName(line.substring(0, i).c_str());
            setBluetoothPassword(line.substring(i).c_str());
          } else if (line == "reset") {
            resetBluetooth();
          } else {
            bluetooth.println("failed");
            break;
          }
          bluetooth.println("success");
          break;
        }
      case 4:  //events
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          Event event = events[line.toInt()];
          line = bluetooth.readStringUntil('\n');
          line.trim();
          event.setState(line.toInt());
          bluetooth.println("success");
          break;
        }
      case 5:  //query
        {
          line = bluetooth.readStringUntil('\n');
          line.trim();
          if (line.equals("timeout")) {
            for (int i = 0; i < timeouts.size(); i++) {
              Timeout timeout = timeouts.get(i);
              bluetooth.println(String((timeout.getUnixtime() * 1440 + timeout.getDayTime().hours * 60 + timeout.getDayTime().minutes) * 60) + ";" + timeout.getPort() + ";" + timeout.getState());
            }
          } else if (line.equals("weekly")) {
            for (int i = 0; i < weeklies.size(); i++) {
              Weekly weekly = weeklies.get(i);
              bluetooth.println(String(weekly.getDays()) + ";" + weekly.getDayTime().hours + ":" + weekly.getDayTime().minutes + ";" + weekly.getPort() + ";" + weekly.getState());
            }
          } else if (line.equals("schedule")) {
            for (int i = 0; i < schedules.size(); i++) {
              Scheduled schedule = schedules.get(i);
              bluetooth.println(String((schedule.getUnixtime() * 1440 + schedule.getDayTime().hours * 60 + schedule.getDayTime().minutes) * 60) + ";" + schedule.getPort() + ";" + schedule.getState());
            }
          } else {
            bluetooth.println("failed");
            break;
          }
          bluetooth.println("success");
          break;
        }
    }
    changed = true;
    lastUpdateTime = 0;
    goto start;
  }
  if (changed) saveData(13);
}
const char* getBluetoothName() {
  digitalWrite(BLUETOOTH_KEY_PIN, HIGH);
  bluetooth.print("AT-NAME?");
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) line = bluetooth.readStringUntil('\n');
  line.trim();
  digitalWrite(BLUETOOTH_KEY_PIN, LOW);
  if (line.length() == 0) return "unknown";
  else {
    return line.substring(line.indexOf(':')).c_str();
  }
}

const char* getBluetoothPassword() {
  digitalWrite(BLUETOOTH_KEY_PIN, HIGH);
  bluetooth.print("AT-PSWD?");
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) line = bluetooth.readStringUntil('\n');
  line.trim();
  digitalWrite(BLUETOOTH_KEY_PIN, LOW);
  if (line.length() == 0) return "unknown";
  else {
    return line.substring(line.indexOf(':')).c_str();
  }
}

bool setBluetoothName(const char* name) {
  digitalWrite(BLUETOOTH_KEY_PIN, HIGH);
  bluetooth.print(String("AT-NAME=") + name);
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) line = bluetooth.readStringUntil('\n');
  line.trim();
  digitalWrite(BLUETOOTH_KEY_PIN, LOW);
  if (line.equals("OK")) return true;
  else return false;
}

bool setBluetoothPassword(const char* pass) {
  digitalWrite(BLUETOOTH_KEY_PIN, HIGH);
  bluetooth.print(String("AT-PSWD=") + pass);
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) line = bluetooth.readStringUntil('\n');
  line.trim();
  digitalWrite(BLUETOOTH_KEY_PIN, LOW);
  if (line.equals("OK")) return true;
  else return false;
}

bool resetBluetooth() {
  digitalWrite(BLUETOOTH_KEY_PIN, HIGH);
  bluetooth.print("AT-ORGL");
  String line = bluetooth.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) line = bluetooth.readStringUntil('\n');
  line.trim();
  digitalWrite(BLUETOOTH_KEY_PIN, LOW);
  if (line.equals("OK")) return true;
  else return false;
}

void fixData() {
  saveData(13);
}

void log(String message, unsigned int level) {
  Serial.println(String("> Level ")+level+": "+message);
}

signed int diffTimeOut(Timeout timeout, DayTime dayTime, unsigned int unixtime) {
  int diff = (unixtime - timeout.getUnixtime()) * 1440 + (dayTime.hours - timeout.getDayTime().hours) * 60 + (dayTime.minutes - timeout.getDayTime().minutes);
  return diff;
}

signed int diffEvent(Event event, DayTime dayTime, unsigned int unixtime) {
  DayTime eventDayTime = event.getDayTime();
  unsigned int eventUnixtime = event.getUnixtime();
  signed int diff = (unixtime - eventUnixtime) * 1440 + (dayTime.hours - eventDayTime.hours) * 60 + dayTime.minutes - eventDayTime.minutes;
  if (diff < 0) return -2;  //bad record
  return diff;
}

signed int diffWeekly(Weekly weekly, DayTime dayTime, unsigned int day) {
  int diff = (dayTime.hours - weekly.getDayTime().hours) * 60 + (dayTime.minutes - weekly.getDayTime().minutes);
  unsigned int diffDays = 0;
  while (true) {
    if (weekly.getDay(day)) {
      diff += diffDays * 1440;
      return diff;
    }
    if (diffDays >= 6) return -2;
    if (day > 0) day--;
    else day = 6;
    diffDays++;
  }
}

DiffValue diffScheduled(Scheduled scheduled, DayTime dayTime, unsigned int unixtime) {
  int diff = (unixtime - scheduled.getUnixtime()) * 1440 + (dayTime.hours - scheduled.getDayTime().hours) * 60 + (dayTime.minutes - scheduled.getDayTime().minutes);
  if (diff < 0) return DiffValue(-1, false);

  unsigned int daysGap = floor(diff / 1440);
  unsigned int count = floor(daysGap / scheduled.getIntervalDays());
  bool lastTime = false;

  if (count < scheduled.getCount()) {
    diff -= 1440 * scheduled.getIntervalDays() * count;
  } else {
    diff -= 1440 * scheduled.getIntervalDays() * scheduled.getCount();
    lastTime = true;
  }
  return DiffValue(diff, lastTime);
}