# HomeAssistant Smart Home System

A comprehensive smart home monitoring system with C++ backend, React frontend, and Discord bot integration, specifically designed for extreme Finnish winter conditions.

## Core Features
- ✅ Four-tier security modes (Residential/Commercial/High Security/Critical Infrastructure)
- ✅ Multi-source data integration (Home Assistant + Pico sensors + Virtual data)
- ✅ Intelligent alerting system (seasonal and geographical awareness)
- ✅ Discord bidirectional communication (command control + alert notifications)
- ✅ Graceful degradation (automatic fallback during sensor failures)

## Technology Stack
- **Backend**: C++17 (cpprestsdk, Paho MQTT)
- **Frontend**: React + Recharts
- **Message Queue**: MQTT (Mosquitto)
- **Home Automation**: Home Assistant
- **Communication**: Discord Webhook/Bot API

## Quick Start
```bash
# Clone the repository
git clone <your-repo-url>
cd HomeAssistant-SmartHome

# Start backend
cd backend
mkdir build && cd build
cmake .. && make
./smart_home_server

# Start frontend
cd frontend
npm install
npm start
```

## Project Structure
```
HomeAssistant-SmartHome/
├── backend/          # C++ backend service
├── frontend/         # React dashboard
├── docs/             # Documentation
├── data/             # Data storage
├── scripts/          # Automation scripts
└── tests/            # Test cases
```

## Configuration Guide
1. Copy `backend/config/example_*.json` to actual config files
2. Create a Discord Bot application and obtain Token
3. Configure Home Assistant long-lived access token
4. Set up MQTT Broker connection details

## License
MIT License
