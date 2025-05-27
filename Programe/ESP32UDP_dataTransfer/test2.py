import tkinter as tk
from threading import Thread
import socket

# TCP setup (changed from UDP to TCP)
TCP_IP = "192.168.10.189"  # Replace with your ESP32 IP address
TCP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # TCP Socket
try:
    sock.connect((TCP_IP, TCP_PORT))
    print(f"Connected to {TCP_IP} on port {TCP_PORT}")
except Exception as e:
    print(f"Error connecting to {TCP_IP}:{TCP_PORT} - {e}")

# Function to handle key press event
def on_key_press(event):
    key = event.keysym.upper()  # Get the key pressed and convert to uppercase
    print(f"Sending command: {key}")
    sock.send(key.encode() + b'\n')  # Send the key press to ESP32
    if key in key_labels:
        key_labels[key].config(bg="black")  # Change the label background to black (lit up)

# Function to handle key release event
def on_key_release(event):
    key = event.keysym.upper()
    print(f"Sending command: P for pause/stop")
    sock.send('P'.encode() + b'\n')  # Send a "P" for pause/stop to ESP32
    if key in key_labels:
        key_labels[key].config(bg="SystemButtonFace")  # Change the label background back to default

# Function to listen for incoming messages from ESP32
def listen_for_messages():
    while True:
        try:
            data = sock.recv(1024)  # Buffer size is 1024 bytes
            if data:
                message = data.decode('utf-8')
                print(f"Received from ESP32: {message}")  # Debugging message received
                # Update the UI with the message from ESP32
                root.after(0, update_label, message)
        except Exception as e:
            print("Error receiving data:", e)
            break

# Function to update the label with the received message
def update_label(message):
    test_label.config(text=message)

# Create the main window
root = tk.Tk()
root.title("WASD Keyboard Controller")
root.geometry("640x480")
root.configure(bg="grey")

# Create a frame to hold the keys
frame = tk.Frame(root, bg="grey")
frame.pack(expand=True)

# Create the labels for "W", "A", "S", and "D"
key_labels = {
    "W": tk.Label(frame, text="W", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace"),
    "A": tk.Label(frame, text="A", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace"),
    "S": tk.Label(frame, text="S", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace"),
    "D": tk.Label(frame, text="D", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace"),
}

# Layout the labels like a keyboard
key_labels["W"].grid(row=0, column=1, padx=5, pady=5)
key_labels["A"].grid(row=1, column=0, padx=5, pady=5)
key_labels["S"].grid(row=1, column=1, padx=5, pady=5)
key_labels["D"].grid(row=1, column=2, padx=5, pady=5)

# Label to display messages from ESP32
test_label = tk.Label(frame, text="Waiting for message...", font=("Arial", 18), relief="raised", bg="SystemButtonFace")
test_label.grid(row=2, column=1, columnspan=3, pady=20)

# Bind key press and release events
root.bind("<KeyPress>", on_key_press)
root.bind("<KeyRelease>", on_key_release)

# Start a new thread to listen for messages from ESP32
listener_thread = Thread(target=listen_for_messages, daemon=True)
listener_thread.start()

# Start the Tkinter main loop
root.mainloop()
