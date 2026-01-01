import serial
import struct
import wave
import os

PORT = "COM6"
BAUD = 115200
SAMPLE_RATE = 8000
CHANNELS = 1
SAMPLE_WIDTH = 2  # int16 = 2 bytes

OUTPUT_PREFIX = "output_"
OUTPUT_DIR = "records"

os.makedirs(OUTPUT_DIR, exist_ok=True)

ser = serial.Serial(PORT, BAUD, timeout=1)
ser.setDTR(False)
ser.setRTS(False)

print("Waiting for AUDIO stream...")

def read_exact(n):
    data = b''
    while len(data) < n:
        chunk = ser.read(n - len(data))
        if not chunk:
            continue
        data += chunk
    return data

file_index = 1

try:
    while True:
        # Tìm magic bytes "AUDIO"
        header = ser.read(5)
        if header != b"AUDIO":
            continue

        # Đọc size (uint32 little-endian)
        size_bytes = read_exact(4)
        data_size = struct.unpack("<I", size_bytes)[0]

        print(f"Receiving frame #{file_index}: {data_size} bytes")

        # Đọc raw PCM
        audio_data = read_exact(data_size)

        # Tạo tên file tăng dần
        filename = f"{OUTPUT_PREFIX}{file_index:04d}.wav"
        filepath = os.path.join(OUTPUT_DIR, filename)

        # Ghi WAV
        with wave.open(filepath, "wb") as wf:
            wf.setnchannels(CHANNELS)
            wf.setsampwidth(SAMPLE_WIDTH)
            wf.setframerate(SAMPLE_RATE)
            wf.writeframes(audio_data)

        print(f"Saved {filepath}\n")

        print("Waiting for AUDIO stream...")
        file_index += 1

except KeyboardInterrupt:
    print("\nStopped by user (Ctrl+C)")
finally:
    ser.close()
