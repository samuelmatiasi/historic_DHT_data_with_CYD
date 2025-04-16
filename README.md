# 📊 LVGL Sensor Dashboard

A touch-enabled graphical dashboard using **LVGL**, **TFT_eSPI**, and **XPT2046** libraries to display real-time and historical **temperature** and **humidity** readings from a **DHT11 sensor** on an **ESP32**.


## 🛠️ Features

- 📟 Real-time sensor visualization using animated arcs
- 📈 Historical charts with scrollable data
- 🖐️ Touchscreen interaction to toggle views
- 🧠 Efficient data handling via circular buffers
- ⚡ Built with [LVGL](https://lvgl.io/) for high-performance embedded GUIs

---

## 📦 Hardware Requirements

- ESP32 board  
- DHT11 temperature & humidity sensor  
- 2.8" or 3.2" TFT display with **XPT2046** touchscreen controller  
- Optional: Breadboard, jumpers, 5V power source

---

## 📚 Libraries Used

Make sure to install the following libraries via PlatformIO or Arduino Library Manager:

- [`lvgl`](https://github.com/lvgl/lvgl)
- [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI)
- [`XPT2046_Touchscreen`](https://github.com/PaulStoffregen/XPT2046_Touchscreen)
- [`DHT sensor library`](https://github.com/adafruit/DHT-sensor-library)

---

## 🧩 Pin Configuration

| Component        | Pin        |
|------------------|------------|
| DHT11 Data       | GPIO 27    |
| XPT2046 CS       | GPIO 33    |
| XPT2046 IRQ      | GPIO 36    |
| XPT2046 MOSI     | GPIO 32    |
| XPT2046 MISO     | GPIO 39    |
| XPT2046 CLK      | GPIO 25    |

---

## 📋 How It Works

- **Arcs Screen:** Displays live temperature and humidity using animated arc gauges.
- **Chart Screens:** Two additional screens show historical values for temperature and humidity.
- **Touch to Switch:** Tap the screen to cycle through the 3 views:
  1. Arcs with current readings  
  2. Temperature chart  
  3. Humidity chart  

The touch input is handled with debounce logic to avoid repeated toggling.

---

## 🧠 Data Handling

- Readings are taken every 2 seconds using `millis()`-based timing.
- Temperature and humidity values are stored in fixed-length arrays (`MAX_DATA_POINTS`).
- The arrays work as circular buffers to keep memory usage stable.
- Historical data is visualized using LVGL's `lv_chart`.

---

## 🖥️ Screenshots

![image](https://github.com/user-attachments/assets/5fc9b330-c2c2-496d-8019-d7c0f57619fa)
![image](https://github.com/user-attachments/assets/e9d7ee6e-7b4b-4d89-baa0-588ccd13ed4c)
![image](https://github.com/user-attachments/assets/9146999e-0a50-4583-9b27-0549d5779a69)




---

## 🧪 Example Snippet

```cpp
temp_data[data_index] = t;
humi_data[data_index] = h;
data_index = (data_index + 1) % MAX_DATA_POINTS;
