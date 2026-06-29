import time
import serial
import sys
from gpiozero import DigitalOutputDevice, Button

# ========================= CONFIGURATION =========================
# 6 Rows (outputs) and 11 Columns (inputs with pull-ups) = 66 possible keys
# ~ ROW_PINS = [2, 3, 4, 17, 27, 22]    
# ~ COL_PINS = [10, 9, 11, 5, 6, 13, 19, 26, 21, 20, 16] 

ROW_PINS = [8, 9, 7, 0, 2, 3]    
COL_PINS = [15, 16, 1, 4, 5, 6, 10, 11, 26, 27, 28] 

# Serial output configuration
SERIAL_PORT = '/dev/serial0'  
BAUD_RATE = 115200

# Debounce time (ms)
DEBOUNCE_MS = 20

# 61-key musical keyboards traditionally start at C2 (MIDI Note 36)
START_MIDI_NOTE = 36  

# Helper arrays to convert MIDI note numbers to musical note names
NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

def get_note_info(row, col):
    """Calculates the unique sequential key ID, MIDI note, and Note Name."""
    key_id = (row * 11) + col
    midi_note = START_MIDI_NOTE + key_id
    
    # Calculate note name and octave
    note_name = NOTE_NAMES[midi_note % 12]
    octave = (midi_note // 12) - 1
    full_note_string = f"{note_name}{octave}"
    
    return key_id, midi_note, full_note_string

# ================================================================

# Setup GPIO using gpiozero
rows = [DigitalOutputDevice(pin, active_high=False, initial_value=True) for pin in ROW_PINS]
cols = [Button(pin, pull_up=True, bounce_time=None) for pin in COL_PINS]

# Open serial port
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Serial opened on {SERIAL_PORT} at {BAUD_RATE} baud")
except Exception as e:
    print(f"Failed to open serial: {e}")
    print("Continuing without serial output...")
    ser = None

# State tracking arrays
last_state = [[False for _ in range(11)] for _ in range(6)]
debounce_timer = [[0 for _ in range(11)] for _ in range(6)]

print("61-Note Musical Keyboard Matrix Scanner active. Press Ctrl+C to exit.")

try:
    while True:
        current_time = time.time() * 1000
        
        for r in range(6):
            rows[r].on() # Activate row (LOW)
            time.sleep(0.001)  
            
            for c in range(11):
                key_id, midi_note, note_name = get_note_info(r, c)
                
                # A 61-key matrix using a 6x11 grid has 66 possible slots.
                # Ignore the 5 unused matrix spots at the very end.
                if key_id >= 61:
                    continue
                    
                key_pressed = cols[c].is_pressed  
                
                # Debounce and change detection
                if key_pressed != last_state[r][c]:
                    if current_time - debounce_timer[r][c] > DEBOUNCE_MS:
                        last_state[r][c] = key_pressed
                        debounce_timer[r][c] = current_time
                        
                        if key_pressed:
                            # Format: Action, MIDI Number, Note Name
                            event = f"NOTE_ON:{midi_note}:{note_name}\n"
                            print(f"Note On  -> {note_name} (MIDI {midi_note})")
                        else:
                            event = f"NOTE_OFF:{midi_note}:{note_name}\n"
                            print(f"Note Off -> {note_name} (MIDI {midi_note})")
                        
                        # Send over serial
                        if ser and ser.is_open:
                            try:
                                ser.write(event.encode('utf-8'))
                            except:
                                pass
                else:
                    debounce_timer[r][c] = current_time
        
            rows[r].off() # Deactivate row (HIGH)
        
        time.sleep(0.005)  

except KeyboardInterrupt:
    print("\nExiting...")
finally:
    if ser and ser.is_open:
        ser.close()
