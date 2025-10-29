# ESP32 Uptime Monitor

A simple uptime monitoring solution for ESP32 with TFT touchscreen display. Monitor multiple websites in real-time with visual status indicators.
<br> Project Link: https://github.com/singh47/esp32-uptime-monitor

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![Status](https://img.shields.io/badge/status-active-success.svg)

![uptime_monitor](https://github.com/user-attachments/assets/12cf8f39-5652-46c9-83c1-6ec7b6d505a0)

## Features

- **Multi-Website Monitoring** - Monitor up to 6 websites simultaneously
- **Individual Check Frequencies** - Set different check intervals for each website
- **Visual Status Indicators** - Green/red circles show website availability at a glance
- **Statistics** - View response times, HTTP codes, and success rates
- **Touch Interface** - Scroll through websites and force manual checks with touch gestures
- **Low Power** - Optimized display updates every 5 seconds to reduce power consumption

## Hardware Requirements

- **ESP32 Development Board** (tested on Cheap Yellow Display - CYD)
- **TFT Display** (320x240 pixels, ILI9341 or compatible)
- **XPT2046 Touchscreen** controller
- **WiFi Connection**

### Pin Configuration (Cheap Yellow Display)

| Component | Pin |
|-----------|-----|
| LCD Backlight | GPIO 21 |
| Touch IRQ | GPIO 36 |
| Touch MOSI | GPIO 32 |
| Touch MISO | GPIO 39 |
| Touch CLK | GPIO 25 |
| Touch CS | GPIO 33 |

## Software Dependencies

Install these libraries through Arduino IDE Library Manager:

- **TFT_eSPI** - Display driver
- **XPT2046_Touchscreen** - Touch controller
- **WiFi** - Built-in ESP32 library
- **HTTPClient** - Built-in ESP32 library

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/singh47/esp32-uptime-monitor.git
cd esp32-uptime-monitor
```

### 2. Install and Configure TFT_eSPI Library
#### Install
<img width="652" height="526" alt="select-TFT_eSPI-arduino" src="https://github.com/user-attachments/assets/9cfd391c-a311-48b6-9506-878d97221a50" />


#### Configure

Edit `TFT_eSPI/User_Setup.h` or create `User_Setup_Select.h` to match your display configuration. (Follow the pin configuration section above). This configuration depends on the type of board you are using. 

### 3. Update WiFi Credentials

Open the `.ino` file and update:

```cpp
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASS "your-wifi-password"
```

### 4. Configure Websites to Monitor

Edit the `websites[]` array:

```cpp
WebsiteConfig websites[] = {
  {"https://www.example.com", "Example", 30},    // Check every 30 seconds
  {"https://www.google.com", "Google", 60},      // Check every 60 seconds
  {"https://www.github.com", "GitHub", 45},      // Check every 45 seconds
  // Add more websites as needed
};
```

Parameters:
- **URL** - Full website URL (must include https://)
- **Display Name** - Short name shown on screen
- **Check Frequency** - How often to check (in seconds)

### 5. Upload to ESP32

1. Select your ESP32 board in Arduino IDE
2. Choose the correct COM port
3. Click Upload

## Usage

### Display Information

The monitor shows for each website:
- **Status Circle** - Green (online) or Red (offline)
- **Response Time** - Milliseconds for last check
- **HTTP Code** - Last response code received
- **Success Rate** - Percentage of successful checks
- **Next Check** - Countdown to next check

### Touch Controls

- **Swipe Up** (top area) - Scroll up through website list
- **Swipe Down** (bottom area) - Scroll down through website list
- **Tap Middle** - Force immediate check of all websites

### Serial Monitor Output

Connect to Serial Monitor (115200 baud) to see:
- WiFi connection status
- Check results for each website
- HTTP response codes and times
- Touch input debugging

## Configuration Options

### Change Number of Websites

```cpp
#define MAX_WEBSITES 10  // Maximum websites to monitor

WebsiteConfig websites[] = {
  // Your websites here
};

int numWebsites = 10;  // Actual number of websites
```

### Adjust Display Update Rate

```cpp
// In loop(), change 5000 to desired milliseconds
if (millis() - lastDisplayUpdate >= 5000) {
```

### Modify Screen Layout

```cpp
int maxVisibleSites = 5;  // Number of sites visible at once
```

### Adjust Backlight Brightness

```cpp
ledcWrite(0, 2048);  // 0-4095, where 2048 = 50% brightness
```

## Success Criteria

Websites are considered "UP" when they return HTTP status codes:
- **2xx** - Success (200 OK, 201 Created, etc.)
- **3xx** - Redirection (301, 302, etc.)

Any other codes (4xx, 5xx) or connection failures mark the site as "DOWN".

## Troubleshooting

### WiFi Won't Connect
- Verify SSID and password are correct
- Check if ESP32 is in range of your router
- Ensure your WiFi is 2.4GHz (ESP32 doesn't support 5GHz)

### Display Not Working
- Check TFT_eSPI configuration matches your display
- Verify pin connections
- Try adjusting backlight brightness

### Touch Not Responding
- Calibrate touch thresholds in code based on Serial Monitor output
- Check touch controller wiring
- Verify XPT2046 library is installed

### Websites Show as Down
- Check if websites require authentication
- Verify ESP32 has internet access
- Try increasing timeout value (default 10 seconds)
- Some websites block automated requests

### Memory Issues
- Reduce `MAX_WEBSITES` if experiencing crashes
- Decrease check frequencies to reduce load

## Performance Tips

- Set longer check frequencies for stable websites
- Stagger check times to avoid simultaneous requests
- Use local/nearby servers when possible for faster responses
- Disable Serial debugging for production use

## Future Enhancements

Potential features for future versions:
- [ ] WiFi configuration via web interface
- [ ] Alert notifications (buzzer, LED)
- [ ] Email/webhook notifications on downtime
- [ ] JSON configuration file
- [ ] Deep sleep mode for battery operation


## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments
- Built with Arduino framework for ESP32
- Uses TFT_eSPI library by Bodmer
- +1 to CY Displays

## Support
If you find this project helpful, please consider:
- Stars!
- Contributing improvements
- Reporting bugs and issues

## Author
**Harman Singh**
- GitHub: [@singh47](https://github.com/singh47)
- Project Link: [https://github.com/singh47/esp32-uptime-monitor](https://github.com/singh47/esp32-uptime-monitor)

---

Made with ❤️ for the ESP32 community
