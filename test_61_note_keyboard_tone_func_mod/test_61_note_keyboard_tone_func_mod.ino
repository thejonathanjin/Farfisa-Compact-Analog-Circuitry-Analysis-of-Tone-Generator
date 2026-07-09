// 61-Key Matrix Keyboard → Live Piano Mode
#include "pitches.h"

const int SPEAKER = 9;

// --- Keyboard Config ---
const byte NUM_ROWS = 6;
const byte NUM_COLS = 11;
const byte TOTAL_KEYS = 61;
const byte START_MIDI_NOTE = 36; // MIDI Note 36 is C2

const byte rowPins[NUM_ROWS] = {2, 3, 4, 5, 6, 7};
const byte colPins[NUM_COLS] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43};

bool keyState[NUM_ROWS][NUM_COLS];
bool lastKeyState[NUM_ROWS][NUM_COLS];
const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// --- Live Piano Note Mapping ---
// An array mapping the 61 keys (starting at C2) to their corresponding frequencies in pitches.h
const int pianoNotes[TOTAL_KEYS] = {
  NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2, // Octave 2
  NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3, // Octave 3
  NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4, // Octave 4 (Middle C is C4)
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5, // Octave 5
  NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6, // Octave 6
  NOTE_C7                                                                                                          // Key 61 (C7)
};

// Track the currently active key index to know when to shut up
int activeKeyIndex = -1; 

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("61-Key Keyboard → Live Piano Mode Ready!");

  // Keyboard setup
  for (byte r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  for (byte c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  // Initialize key states
  for (byte r = 0; r < NUM_ROWS; r++) {
    for (byte c = 0; c < NUM_COLS; c++) {
      keyState[r][c] = false;
      lastKeyState[r][c] = false;
    }
  }
}

void loop() {
  scanMatrix();
  checkStateChanges();
  delay(1);
}

// ==================== Keyboard Functions ====================
void scanMatrix() {
  for (byte c = 0; c < NUM_COLS; c++) {
    digitalWrite(colPins[c], LOW);
    delayMicroseconds(10);
    
    for (byte r = 0; r < NUM_ROWS; r++) {
      keyState[r][c] = (digitalRead(rowPins[r]) == LOW);
    }
    digitalWrite(colPins[c], HIGH);
  }
}

void checkStateChanges() {
  for (byte r = 0; r < NUM_ROWS; r++) {
    for (byte c = 0; c < NUM_COLS; c++) {
      int keyIndex = (r * NUM_COLS) + c;
      if (keyIndex >= TOTAL_KEYS) continue;

      byte midiNote = START_MIDI_NOTE + keyIndex;

      // Key newly pressed
      if (keyState[r][c] && !lastKeyState[r][c]) {
        printNoteEvent(midiNote, true);
        
        // Play the live note matching this key index instantly
        tone(SPEAKER, pianoNotes[keyIndex]);
        activeKeyIndex = keyIndex; // Track that this key is currently making noise
        
        lastKeyState[r][c] = true;
      } 
      // Key released
      else if (!keyState[r][c] && lastKeyState[r][c]) {
        printNoteEvent(midiNote, false);
        
        // Only turn off the speaker if the released key was the one playing
        if (activeKeyIndex == keyIndex) {
          noTone(SPEAKER);
          activeKeyIndex = -1;
        }
        
        lastKeyState[r][c] = false;
      }
    }
  }
}

void printNoteEvent(byte midiNote, bool isPressed) {
  int noteIndex = midiNote % 12;
  int octave = (midiNote / 12) - 1;

  Serial.print("Note: ");
  Serial.print(noteNames[noteIndex]);
  Serial.print(octave);
  Serial.println(isPressed ? " [ON]" : " [OFF]");
}