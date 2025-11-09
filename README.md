# TelegramHomeControl
A smart home automation system powered by ESP8266 and controlled through a Telegram bot with smooth interface button system. It monitors temperature, humidity, and rain, and lets you control lights, fan speed, and a servo hood remotely.

 complete 👇
🏠 TelegramHomeControl


A smart home automation project using ESP8266 and a Telegram Bot.
Monitor and control your home devices remotely - lights, fan, servo hood - while tracking temperature, humidity, and rain in real time.
Fully interactive Telegram keyboard and buzzer alerts for weather and system updates.

---

🚀 Features

 🌡️ Monitor temperature & humidity via DHT11
 🌧️ Detect rain and auto-control servo hood
 💡 Control two lights (ON/OFF)
 🌀 Control fan with 3 speed levels
 🔔 Buzzer tones for alerts & commands
 📱 Telegram bot control with inline keyboard
 🔁 Auto Wi-Fi reconnection and live feedback
 🎵 Plays random melodies on command

---

🧠 Components Used

| Component          | Model                  |
| ------------------ | ---------------------- |
| Microcontroller    | ESP8266 NodeMCU        |
| Temperature Sensor | DHT11                  |
| Rain Sensor        | FC-37 or YL-83         |
| Servo Motor        | SG90                   |
| Buzzer             | Passive                |
| Fan/Relay          | 5V Relay or MOSFET     |
| LEDs/Lights        | Any 5V light or module |

---

⚙️ Pin Configuration

| Function     | Pin |
| ------------ | --- |
| DHT11 Sensor | D4  |
| Rain Sensor  | D5  |
| Buzzer       | D6  |
| Servo Motor  | D7  |
| Light 1      | D2  |
| Light 2      | D3  |
| Fan          | D8  |

---

🧩 How It Works

1. ESP8266 connects to your Wi-Fi network.
2. Telegram bot sends a control panel keyboard to your chat.
3. You can toggle lights, fan, and hood directly from Telegram.
4. Auto-alerts are sent for high/low temperature or when it rains.
5. Buzzer plays different tones for each event.

---

💬 Telegram Commands

| Command                 | Action                      |
| ----------------------- | --------------------------- |
| /start                  | Opens control panel         |
| Temperature             | Shows current temperature   |
| Humidity                | Shows current humidity      |
| Status                  | Displays all readings       |
| Weather                 | Shows rain or dry condition |
| Light 1 / Light 2       | Toggle lights               |
| Fan On / Fan Off        | Turn fan on/off             |
| Fan Low / Medium / High | Adjust speed                |
| Open Hood / Close Hood  | Control servo hood          |
| Music                   | Play random melody          |

---

🧾 License

This project is licensed under the MIT License - free to use, modify, and share with credit.

---
You have to know about "how to create 'telegram bot' and what your (chat ID)"
