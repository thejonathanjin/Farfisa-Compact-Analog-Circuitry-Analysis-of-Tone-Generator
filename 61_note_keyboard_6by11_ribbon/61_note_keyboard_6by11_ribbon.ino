//Plays a song on a speaker

#include "pitches.h" //Header file with pitch definitions

const int SPEAKER=9;  //Speaker Pin

// --- Configuration Constants ---
const byte NUM_ROWS = 6;
const byte NUM_COLS = 11;
const byte TOTAL_KEYS = 61;
const byte START_MIDI_NOTE = 36; // C2 on standard keyboards

// --- Pin Assignments ---
const byte rowPins[NUM_ROWS] = {2, 3, 4, 5, 6, 7};
const byte colPins[NUM_COLS] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43};

// --- State Tracking Arrays ---
bool keyState[NUM_ROWS][NUM_COLS];     // Current debounced state (true = pressed)
bool lastKeyState[NUM_ROWS][NUM_COLS]; // History state from previous scan cycle

// Array of text labels for the 12 chromatic scale notes
const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

//Note Array to comment out later
int notes[] = {
 NOTE_A4, NOTE_E3, NOTE_A4, 0, 
 NOTE_A4, NOTE_E3, NOTE_A4, 0,
 NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_C4, NOTE_D4,
 NOTE_E4, NOTE_E3, NOTE_A4, 0
};

//The Duration of each note (in ms)
int times[] = {
 250, 250, 250, 250, 
 250, 250, 250, 250,
 125, 125, 125, 125, 125, 125, 125, 125,
 250, 250, 250, 250 
};

void setup() {
  // Use 115200 for readable text output in the Arduino Serial Monitor
  Serial.begin(115200); 
  while (!Serial) { ; } // Wait for serial port to connect (needed for native USB boards)
  Serial.println("61-Key Matrix Reader Initialized.");

  // Configure Rows as Inputs with internal Pull-up Resistors
  for (byte r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }

  // Configure Columns as Outputs and set them HIGH idle
  for (byte c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  // Zero-out our tracking matrix variables
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
  delay(1); // Small delay to handle fundamental mechanical contact bounce
}

// Sequentially drives columns LOW and reads the resulting row inputs
void scanMatrix() {
  for (byte c = 0; c < NUM_COLS; c++) {
    digitalWrite(colPins[c], LOW); // Activate target column
    
    // Tiny settling delay for capacitance on long matrix ribbon cables
    delayMicroseconds(10); 
    
    for (byte r = 0; r < NUM_ROWS; r++) {
      // A LOW state read means the key switch is actively closed/pressed
      keyState[r][c] = (digitalRead(rowPins[r]) == LOW);
    }
    
    digitalWrite(colPins[c], HIGH); // Deactivate column
  }
}

// Evaluates current vs past state to trigger precise Text/Serial events
void checkStateChanges() {
  for (byte r = 0; r < NUM_ROWS; r++) {
    for (byte c = 0; c < NUM_COLS; c++) {
      
      // Calculate active sequential key position inside the 6x11 grid
      int keyIndex = (r * NUM_COLS) + c;
      
      // Ignore calculation overflow beyond the physical 61-key construction limit
      if (keyIndex >= TOTAL_KEYS) continue;

      // Map key position directly to consecutive MIDI Note Numbers
      byte midiNote = START_MIDI_NOTE + keyIndex;

      // Condition A: Key newly pressed down
      if (keyState[r][c] == true && lastKeyState[r][c] == false) {
        printNoteEvent(midiNote, true);
        lastKeyState[r][c] = true;
      }
      // Condition B: Key newly released
      else if (keyState[r][c] == false && lastKeyState[r][c] == true) {
        printNoteEvent(midiNote, false);
        lastKeyState[r][c] = false;
      }
      
    }
  }
}

// Decodes MIDI number into Note name + Octave and prints to Serial Monitor
void printNoteEvent(byte midiNote, bool isPressed) {
  // Calculate index (0-11) within the chromatic octave scale
  int noteIndex = midiNote % 12;
  
  // Calculate the specific mathematical octave number
  int octave = (midiNote / 12) - 1;

  // Format and send text string over serial link
  Serial.print("Note: ");
  Serial.print(noteNames[noteIndex]);
  Serial.print(octave);
  
  if (isPressed) {
    Serial.println(" [ON]");
  } else {
    Serial.println(" [OFF]");
  }
}
