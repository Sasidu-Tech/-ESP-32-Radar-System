📡 ESP32 Web-Based Radar Detection System

A real-time ESP32-based Radar Detection System that scans the surrounding area using an ultrasonic sensor mounted on a servo motor and displays detected objects on a live web-based radar interface.

The system also uses a laser module to indicate the detected object, creating a simple interactive radar experience.

---

🚀 Project Overview

This project combines embedded hardware with a web-based visualization system.

The ESP32 controls the servo motor and ultrasonic sensor to scan approximately 180°. During the scan, the system measures the distance to objects and sends the angle and distance data to a web interface.

The web page displays the radar data in real time, allowing detected objects to be visualized based on their angle and distance.

✨ Main Features

- 📡 Real-time radar scanning
- 🔄 180° servo-based scanning
- 📏 Ultrasonic distance measurement
- 🎯 Object detection
- 🔴 Laser indication for detected objects
- 💻 Real-time web radar visualization
- 📊 Angle and distance display
- 🌐 ESP32 Web Server
- ⚡ Real-time sensor data processing
- 📱 Accessible from devices connected to the same Wi-Fi network

---

🛠️ Hardware Components

Component| Quantity
ESP32 DevKit| 1
HC-SR04 Ultrasonic Sensor| 1
Servo Motor| 1
Laser Module| 1
Jumper Wires| As required
Breadboard| 1
Power Supply| 1

---

💻 Software & Technologies

- Arduino IDE
- C/C++
- ESP32
- HTML
- CSS
- JavaScript
- Web Server
- Ultrasonic Distance Measurement
- Real-Time Data Visualization

---

🔌 System Architecture

              ┌─────────────────────┐
              │       ESP32         │
              │                     │
              │   Web Server        │
              │   Data Processing   │
              └──────────┬──────────┘
                         │
              ┌──────────┴──────────┐
              │                     │
        ┌─────▼─────┐         ┌─────▼─────┐
        │   Servo   │         │  HC-SR04   │
        │   Motor   │         │ Ultrasonic │
        └─────┬─────┘         └─────┬─────┘
              │                     │
              └──────────┬──────────┘
                         │
                  Object Detection
                         │
                  ┌──────▼──────┐
                  │ Laser Module│
                  └─────────────┘

                         │
                         ▼
              ┌─────────────────────┐
              │   Web Radar Page    │
              │                     │
              │ Angle + Distance    │
              │ Real-Time Radar     │
              └─────────────────────┘

---

🔄 How It Works

1. The ESP32 starts the web server and connects to the configured Wi-Fi network.
2. The servo motor moves the ultrasonic sensor across the scanning area.
3. At each angle, the HC-SR04 measures the distance to the nearest detected object.
4. The ESP32 processes the sensor readings.
5. If an object is detected within the configured detection range, the laser module is activated.
6. The ESP32 sends the angle and distance information to the web interface.
7. JavaScript processes the received data.
8. The radar interface displays the detected object at its corresponding angle and distance.
9. The process continuously repeats to provide a real-time radar effect.

---

🌐 Web Radar Interface

The ESP32 hosts a web page containing the radar visualization.

The interface can display:

- 🟢 Radar scanning area
- 📍 Detected object position
- 📐 Detection angle
- 📏 Object distance
- 🔄 Real-time scanning animation
- 📊 Live sensor information

Connect your phone or computer to the same Wi-Fi network as the ESP32 and open the ESP32's IP address in a web browser.

http://ESP32_IP_ADDRESS

Example:

http://192.168.1.100

---

🎯 Object Detection

The ultrasonic sensor continuously measures the distance between the sensor and an object.

Conceptually:

Distance = (Echo Time × Speed of Sound) / 2

The division by 2 is required because the ultrasonic signal travels to the object and returns to the sensor.

---

🔴 Laser Indication

The laser module is used as a visual indicator.

When the system detects an object within the configured detection range, the laser can be activated to indicate the detected direction.

«⚠️ Safety: Never point the laser at people, vehicles, aircraft, or directly into anyone's eyes.»

---

📡 Radar Scanning

The servo motor changes the ultrasonic sensor's direction.

             90°
              │
              │
        45°   │   135°
           \  │  /
            \ │ /
             \│/
        ──────●──────
             ESP32

The system scans from approximately:

0° → 180°

and then returns:

180° → 0°

This continuous movement creates the radar scanning effect.

---

📁 Project Structure

ESP32-Web-Radar/
│
├── ESP32-Web-Radar.ino
├── README.md
│
└── web/
    ├── index.html
    ├── style.css
    └── script.js

If the HTML, CSS and JavaScript are embedded directly inside the Arduino sketch, they can instead be kept in a single ".ino" file.

---

⚙️ Configuration

Before uploading the code, configure your Wi-Fi credentials:

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

Then upload the program to the ESP32 using Arduino IDE.

Open the Serial Monitor and find the ESP32 IP address.

Example:

ESP32 IP Address: 192.168.1.100

Open that IP address in a browser.

---

🧪 Testing

The project was tested by placing objects at different positions and distances.

The system successfully demonstrates:

- Servo scanning
- Ultrasonic distance measurement
- Object detection
- Laser indication
- Real-time angle information
- Real-time distance information
- Web-based radar visualization

---

🔮 Future Improvements

Possible future upgrades include:

- 📱 Mobile-friendly radar interface
- 📈 Distance history graph
- 💾 Data logging
- 🚨 Object detection alerts
- 🔊 Buzzer alerts
- 🎨 Improved radar UI
- 📡 Remote monitoring over the Internet
- 🤖 Multiple ultrasonic sensors
- 📷 ESP32-CAM integration
- 🧠 AI-based object classification
- 📊 Advanced radar analytics

---

👨‍💻 Author

Sasidu-Tech

Developed as an embedded systems and IoT project using ESP32.

---

⭐ Support

If you find this project useful or interesting, consider giving the repository a ⭐.

---

📜 License

This project is open-source and available for educational and personal project purposes.
