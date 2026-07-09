// Merged: 61-Key Matrix Keyboard → Triggers Demo Song
#include "pitches.h"

const int SPEAKER = 9;

// --- Keyboard Config ---
const byte NUM_ROWS = 6;
const byte NUM_COLS = 11;
const byte TOTAL_KEYS = 61;
const byte START_MIDI_NOTE = 36;

const byte rowPins[NUM_ROWS] = {2, 3, 4, 5, 6, 7};
const byte colPins[NUM_COLS] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43};

bool keyState[NUM_ROWS][NUM_COLS];
bool lastKeyState[NUM_ROWS][NUM_COLS];

const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Demo Song (tone sequence)
// int notes[] = {
//   NOTE_A4, NOTE_E3, NOTE_A4, 0, 
//   NOTE_A4, NOTE_E3, NOTE_A4, 0,
//   NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_C4, NOTE_D4,
//   NOTE_E4, NOTE_E3, NOTE_A4, 0
// };

// int times[] = {
//   250, 250, 250, 250, 
//   250, 250, 250, 250,
//   125, 125, 125, 125, 125, 125, 125, 125,
//   250, 250, 250, 250 
// };

int notes[] = {
  NOTE_A4
};

int times[] = {
  250
};

bool songPlaying = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("61-Key Keyboard → Song Trigger Ready!");

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
        
        // Trigger the full song when any key is pressed
        if (!songPlaying) {
          playSong();
        }
        
        lastKeyState[r][c] = true;
      } 
      // Key released
      else if (!keyState[r][c] && lastKeyState[r][c]) {
        printNoteEvent(midiNote, false);
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

// ==================== Song Player ====================
void playSong() {
  songPlaying = true;
  Serial.println("=== Playing Song ===");
  
  for (int i = 0; i < 20; i++) {
    if (notes[i] != 0) {
      tone(SPEAKER, notes[i], times[i]);
    } else {
      noTone(SPEAKER);
    }
    delay(times[i]);
  }
  
  noTone(SPEAKER);
  songPlaying = false;
  Serial.println("=== Song Finished ===");
}