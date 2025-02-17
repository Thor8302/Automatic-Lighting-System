# Automatic Lighting System

## Overview
This project implements an advanced IoT-based automatic lighting system that leverages Embedded Systems, AI/ML, and Cloud Computing. It integrates ESP32-CAM, NodeMCU, and Arduino Nano for communication and data processing, enabling energy-efficient and intelligent lighting control.

## Features
- **Face Detection with AI/ML**: EfficientDet-Lite2 model deployed on AWS EC2 for sub-second face detection, reducing false positives by 20%.
- **Multi-Layer Motion Detection**: Utilizes PIR sensors to enhance energy efficiency by 35% in a 25 m² area.
- **Cloud Integration**: Flask-based REST API containerized with Docker for scalable deployment.
- **ESP32-CAM Firmware Optimization**: Dynamic brightness and contrast adjustments reduce false positives by 40% and improve image clarity under varying lighting conditions.

## System Architecture
![IMG20230306165047](https://github.com/user-attachments/assets/c222be2e-c952-4bec-894b-c55eca0a78a4)

## Hardware Components
- ESP32-CAM
- NodeMCU
- Arduino Nano
- PIR Sensors
- LEDs/Lighting Units

## Software & Technologies Used
- **Embedded C** for microcontroller programming
- **Python & Flask** for server-side processing
- **AWS EC2** for AI model deployment
- **Docker** for containerized cloud integration
- **OpenCV & TensorFlow Lite** for face detection

## Installation & Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/Automatic-Lighting-System.git
   ```
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```
3. Flash the microcontrollers with respective firmware.
4. Deploy the AI model on AWS EC2.
5. Start the Flask server:
   ```bash
   python app.py
   ```

## Code Structure
```
Automatic-Lighting-System/
│── firmware/                # ESP32-CAM, NodeMCU, Arduino Nano firmware
│── models/                  # AI/ML model files
│── server/                  # Flask API and cloud integration
│── images/                  # Placeholder for project images
│── videos/                  # Placeholder for demonstration videos
│── README.md
```

## Demonstration
### Images
![IMG20230306165118 (1)](https://github.com/user-attachments/assets/a26f81bf-d408-4d2f-8e58-f79743c45e75)
![IMG20230306165054](https://github.com/user-attachments/assets/fb3090de-0264-4df0-ad24-12ed0ac05bca)
![IMG20220615074023](https://github.com/user-attachments/assets/6ba14576-32d0-4bad-8277-b7a85b96a106)
![IMG-20240519-WA0002](https://github.com/user-attachments/assets/605b3f5e-9e2c-44d1-996d-4379c0867d38)

### Video Demonstration
[![Watch the video](path/to/video_thumbnail.png)](path/to/project_video.mp4)

## Future Improvements
- Optimize AI model for edge computing to reduce cloud dependency.
- Improve real-time response speed by refining data transmission protocols.
- Enhance security with encrypted communication between devices and the server.

## Contributors
- **Your Name** - Lead Developer

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

