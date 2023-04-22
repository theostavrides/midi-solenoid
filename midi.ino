#include <MIDIUSB.h>
#include <MIDIUSB_Defs.h>
#include <frequencyToNote.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>

const int latchPin = 5; // Goes to SR 12 (ST_CP Storage Reg Clock In)
const int clockPin = 3; // Goes to SR 11 (SH_CP Shift Reg Clock In)
const int dataPin = 10;  // Goes to SR 14 (DS Serial Data Input)

// -------------------------- Shift Register State -----------------------
 
byte shiftRegisterState[] = { 0b00000000, 0b00000000, 0b00000000 };

// --------------------------- Instrument State  -----------------------------

struct Instrument {
  int id;
  bool on;
  unsigned long lastOnEvent;
};

Instrument instruments[20] {
  { id: 0, on: false, lastOnEvent: 0 },  
  { id: 1, on: false, lastOnEvent: 0 },  
  { id: 2, on: false, lastOnEvent: 0 },  
  { id: 3, on: false, lastOnEvent: 0 },  
  { id: 4, on: false, lastOnEvent: 0 },  
  { id: 5, on: false, lastOnEvent: 0 },  
  { id: 6, on: false, lastOnEvent: 0 },  
  { id: 7, on: false, lastOnEvent: 0 },  
  { id: 8, on: false, lastOnEvent: 0 },  
  { id: 9, on: false, lastOnEvent: 0 },  
  { id: 10, on: false, lastOnEvent: 0 },  
  { id: 11, on: false, lastOnEvent: 0 },  
  { id: 12, on: false, lastOnEvent: 0 },  
  { id: 13, on: false, lastOnEvent: 0 },  
  { id: 14, on: false, lastOnEvent: 0 },  
  { id: 15, on: false, lastOnEvent: 0 },
  { id: 16, on: false, lastOnEvent: 0 },  
  { id: 17, on: false, lastOnEvent: 0 },  
  { id: 18, on: false, lastOnEvent: 0 },  
  { id: 19, on: false, lastOnEvent: 0 },
};

// ----------------------------- Shift Register Functions -----------------------------

void pushBytesToSR(){
  digitalWrite(latchPin, LOW);
  
  for (int i = 2; i >= 0; i --) {
    shiftOut(dataPin, clockPin, MSBFIRST, shiftRegisterState[i]);
  }

  digitalWrite(latchPin, HIGH); 
}

// Sets the state of an indidual bit on one of the shift registers
void updateSRFlag(int index, bool on){
  int byteIndex = floor(index / 8);
  int bitIndex = index - (byteIndex * 8);
  bitWrite(shiftRegisterState[byteIndex], bitIndex, on);
}

void updateShiftRegisterState(struct Instrument insts[]) {
  // int size = sizeof(insts)/sizeof(insts[0]);
  // Serial.print(size);
  for (int instrumentId = 0; instrumentId < 20; instrumentId++) {
    updateSRFlag(instrumentId, insts[instrumentId].on);
  }
}

// ------------------------------- Midi Functions ----------------------------

int updateInstrumentState (midiEventPacket_t e){
  if (e.header != 0x9) return;
  int noteId = int(e.byte2);
  instruments[noteId].on = true;
  instruments[noteId].lastOnEvent = millis();
}

void clearStaleInstrumentNotes (int MaxOnTime) {
  for (int instrumentId = 0; instrumentId < 20; instrumentId++) {
    unsigned long now = millis();

    if (instruments[instrumentId].on == true) {
      if (now - instruments[instrumentId].lastOnEvent > MaxOnTime) {
        instruments[instrumentId].on = false;
      }
    }
  }
}


// -------------------------------- Arduino ---------------------------------

void setup()
{
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pushBytesToSR();
}

void loop()
{
  midiEventPacket_t midiEvent = MidiUSB.read();
  updateInstrumentState(midiEvent);
  clearStaleInstrumentNotes(50);
  updateShiftRegisterState(instruments);
  pushBytesToSR();
}