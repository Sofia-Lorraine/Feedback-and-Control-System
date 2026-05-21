import serial
import time
from openai import OpenAI
from langchain_community.chat_message_histories import ChatMessageHistory
import sys

# --- Connect to LM Studio Local server ---
client = OpenAI(
    base_url="http://127.0.0.1:1234/v1",
    api_key="lm-studio"
)

# Initialize the history object
history = ChatMessageHistory()


SERIAL_PORT = 'COM3'  
BAUD_RATE = 9600
MODEL_NAME = "google/gemma-4-e2b" 

def log_event(message):
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    log_entry = f"[{timestamp}] {message}\n"
    print(log_entry.strip())
    with open("security_log.txt", "a") as f:
        f.write(log_entry)

def main():
    try:
        arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Allow hardware connection to settle
        print("Agent Brain Connected to Sentry. Monitoring stream with AI active...")
        print("Waiting for data from Arduino...")
    except Exception as e:
        print(f"Failed to connect to Arduino on {SERIAL_PORT}: {e}")
        return

    while True:
        if arduino.in_waiting > 0:
            try:
                # Read raw line from Arduino
                raw_line = arduino.readline().decode('utf-8', errors='ignore').strip()
                
                # Show exactly what is arriving over the USB wire
                print(f"\n[RAW RECEIVED DATA]: '{raw_line}'")

                # If the line is empty, skip it
                if not raw_line:
                    continue

                # Flexible check: looks for the markers anywhere in the text line
                if "MOTION=" not in raw_line or "MODE=" not in raw_line:
                    print("[INFO] Line ignored: Missing 'MOTION=' or 'MODE=' keys.")
                    continue

                # Parse data stream (Handles spaces or mixed casing)
                # Cleans up string text like: "TIME=2000-01-01T00:17:56 | MOTION=1 | MODE=SECURITY"
                cleaned_line = raw_line.replace(" ", "").upper()
                parts = cleaned_line.split('|')
                
                current_time = "UNKNOWN"
                motion = 0
                mode = "SECURITY"

                for part in parts:
                    if part.startswith("TIME="):
                        current_time = part.split('=')[1]
                    elif part.startswith("MOTION="):
                        motion = int(part.split('=')[1])
                    elif part.startswith("MODE="):
                        mode = part.split('=')[1]

                # Only proceed to AI processing if the sensor reads active motion
                if motion == 1:
                    log_event(f"System Alert: Motion detected at {current_time} under '{mode}' mode. Consulting AI...")

                    # Construct the prompt
                    ai_prompt = (
                        f"ALERT: Motion detected!\n"
                        f"Timestamp: {current_time}\n"
                        f"Current Mode: {mode}\n\n"
                        f"Analyze this context. If the mode is SECURITY, this is a dangerous threat—you MUST respond with 'TRIGGER_ALARM'. "
                        f"If the mode is SAFE, these are normal working hours—respond with 'IGNORE'. "
                        f"Provide a 1-sentence reasoning, followed by your final command on a new line."
                    )

                    history.add_user_message(ai_prompt)

                    # System prompt payload injection
                    messages_with_history = [
                        {
                            "role": "system", 
                            "content": "You are an automated physical security AI brain. Your job is to process sensor data inputs and output exact system commands based on security rules."
                        }
                    ]
                    
                    for m in history.messages:
                        role = "user" if m.type == "human" else "assistant"
                        messages_with_history.append({"role": role, "content": m.content})

                    print("Sending request to LM Studio...")
                    response = client.chat.completions.create(
                        model=MODEL_NAME,
                        messages=messages_with_history,
                        temperature=0.2 
                    )

                    ai_analysis = response.choices[0].message.content
                    log_event(f"AI Analysis:\n{ai_analysis}")
                    
                    history.add_ai_message(ai_analysis)

                    if "TRIGGER_ALARM" in ai_analysis:
                        log_event("CRITICAL: AI authorized threat mitigation. Sending hardware execution signal.")
                        arduino.write(b"TRIGGER_ALARM\n")
                        time.sleep(5) 
                    else:
                        log_event("AI Decision: Event categorized as safe. No action taken.")
                        time.sleep(2) 

            except Exception as parse_err:
                print(f"Stream read/AI error: {parse_err}")
                
        time.sleep(0.1)

if __name__ == "__main__":
    main()
    