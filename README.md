# Multi-Sensor ESP32-Based Monitoring System

## Overview

This project is an ESP32-based multi-sensor monitoring system that integrates several sensors, a servo motor, a display, and communication protocols to collect, display, and transmit data. It supports Bluetooth Low Energy (BLE) and WiFi communication for real-time data sharing and control.

The system monitors environmental data such as temperature, humidity, light levels, and accelerometer values, while providing alerts for anomalous readings. A TFT display is used for local visualization, and BLE is used for wireless communication with connected devices.

---

## Features

- **Sensors Integration:**
  - **Temperature and Humidity**: Using the Adafruit AHTX0 sensor.
  - **Light Intensity**: Using an LDR sensor.
  - **Motion Detection**: Using SparkFun LSM6DSO accelerometer and gyroscope.
  - **Capacitive Touch Sensor**: Adafruit CAP1188 for detecting touch events.

- **Actuators:**
  - **Servo Motor**: Controlled based on touch input.
  - **Buzzer**: Provides audio alerts for abnormal conditions.

- **Communication:**
  - **WiFi**: Connects to a server to upload sensor data and alerts.
  - **Bluetooth Low Energy (BLE)**: Transmits sensor data to nearby devices.

- **Display**: TFT screen displays sensor readings and status.

- **Data Storage**: Non-Volatile Storage (NVS) is used for saving WiFi credentials.

---

## Hardware Requirements

- **ESP32 Dev Board**
- **Adafruit AHTX0** (Temperature and Humidity Sensor)
- **SparkFun LSM6DSO** (Accelerometer and Gyroscope)
- **Adafruit CAP1188** (Capacitive Touch Sensor)
- **LDR Sensor** (Light Intensity Sensor)
- **Servo Motor**
- **Buzzer**
- **TFT Display** (TFT_eSPI)
- Supporting passive components (resistors, wires, breadboard, etc.)

---

## Software Requirements

- **Arduino IDE** with ESP32 Board Manager installed.
- Required libraries (Install via Arduino Library Manager):
  - `Wire`
  - `HttpClient`
  - `WiFi`
  - `TFT_eSPI`
  - `BLEDevice`
  - `Adafruit AHTX0`
  - `SparkFun LSM6DSO`
  - `Servo`

---

## Pin Configuration

| Component           | ESP32 Pin |
|---------------------|-----------|
| LDR Sensor          | GPIO 36   |
| Servo Motor         | GPIO 17   |
| Buzzer              | GPIO 33   |
| TFT Display         | SPI Pins  |
| Touch Sensor        | I2C Pins  |
| AHTX0 Sensor        | I2C Pins  |
| LSM6DSO Sensor      | I2C Pins  |

---

## How to Use

1. **Setup Hardware:**
   - Connect all components as per the pin configuration table.

2. **Configure WiFi Credentials:**
   - Update WiFi credentials in NVS using the `nvs_access()` function.

3. **Upload Code:**
   - Open the code in Arduino IDE.
   - Select the correct board and COM port.
   - Compile and upload the code.

4. **Monitor System:**
   - View real-time data on the TFT display.
   - Use BLE to receive data on a connected device.
   - Check alerts via the buzzer and BLE notifications.

5. **Server Communication:**
   - Ensure the ESP32 can connect to the server (`18.118.15.226` on port `5000`).
   - Real-time data and alerts are sent via HTTP GET requests.

---

## BLE Data Format

- **Real-Time Data**: `Fan-lvl:<level>,LDR:<value>,Temp:<temperature>,Hum:<humidity>,X:<x>,Y:<y>,Z:<z>`
- **Alerts**: `ALERT: <message>`

---

## Functional Highlights

1. **BLE Communication**:
   - Devices can connect to the ESP32 and receive live updates.
   - Notifications are sent for alerts or abnormal sensor readings.

2. **Display Updates**:
   - Displays real-time data, including:
     - Fan level based on touch input.
     - Temperature and humidity.
     - LDR value.
     - Accelerometer readings (X, Y, Z).

3. **Servo Control**:
   - Servo angle adjusts based on touch input (1-8 levels).

4. **Alerts**:
   - Alerts triggered for:
     - Low temperature or humidity.
     - Abnormal motion (X, Y, Z axis).
     - Low light intensity.
   - Audio alert through the buzzer.
   - Notifications via BLE and server.

5. **Server Integration**:
   - Sends data and alert information to a remote server.
  # Server-Side Application: Flask-Based Data Management API

## Server.py

 server.py is a Flask-based server application for managing sensor data received from the ESP32 monitoring system. It supports data storage, retrieval, and visualization.

---

## Features

- **Data Upload**: Receives sensor data via an HTTP GET request and appends it to a CSV file.
- **Data Retrieval**: Serves the latest sensor data through a REST API endpoint.
- **Web Interface**: A simple HTML interface (`index.html`) can be added for visualization or interaction.
- **Error Handling**: Reports errors when parsing or processing data.

---

## Endpoints

### 1. Root Endpoint
- **URL**: `/`
- **Method**: GET
- **Description**: Serves the web interface.
- **Response**: HTML page (requires `index.html` in the templates directory).

### 2. Retrieve Data
- **URL**: `/data`
- **Method**: GET
- **Description**: Retrieves the latest sensor data from the CSV file.
- **Response**:
  - **200 OK**: JSON object containing the latest data.
  - **404 Not Found**: If no data is available.
  - **500 Internal Server Error**: If an error occurs during data retrieval.

### 3. Upload Data
- **URL**: `/api/upload`
- **Method**: GET
- **Parameters**:
  - `statusCode` (int): Indicates the status of the data being sent.
    - `200`: Normal data upload.
    - `500`: Error or alert.
  - `info` (str): Sensor data in the format `Fan-lvl:<value>,LDR:<value>,Temp:<value>,Hum:<value>,X:<value>,Y:<value>,Z:<value>`.
- **Description**: Parses the incoming sensor data, logs it, and appends it to the CSV file (`sensor_data.csv`).
- **Response**:
  - **200 OK**: If data is successfully received and saved.
  - **500 Internal Server Error**: If an error occurs during parsing or saving.

---

## Data File

The server uses a CSV file (`sensor_data.csv`) to store sensor readings. 

### CSV Columns
- `timestamp`: Timestamp of when the data was received.
- `fan_lvl`: Fan level.
- `ldr`: Light sensor reading.
- `temp`: Temperature in degrees Celsius.
- `hum`: Humidity in percentage.
- `x`, `y`, `z`: Accelerometer readings for the X, Y, and Z axes.


## index.html


index.html details the implementation of a real-time data visualization web page for the sensor monitoring system. The page uses the Chart.js library to plot temperature, humidity, and LDR sensor data dynamically.

---

## Features

- **Dynamic Updates**: Automatically fetches the latest sensor data from the server every second.
- **Interactive Charts**: Displays sensor readings (temperature, humidity, and LDR values) on a line chart.
- **Responsive Design**: Adjusts to different screen sizes for better user experience.

---

## How to Use

1. **Place the HTML File**:
   - Save the following code in the `templates` directory as `index.html`.

2. **Access the Page**:
   - Navigate to `http://<server-ip>:5000/` in your browser to view the real-time chart.

3. **Ensure Data Retrieval**:
   - The server must be running and provide sensor data at the `/data` endpoint.
