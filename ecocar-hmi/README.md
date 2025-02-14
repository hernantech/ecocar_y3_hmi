### This readme holds the specification doc for the HMI client/server architecture

# EcoCar HMI System Specification

## Overview
The EcoCar HMI system consists of a Qt/QML frontend application and a Flask backend server, designed to run on the NXP NavQ Plus (i.MX8M Plus) platform. The system provides real-time vehicle data visualization through a hardware-accelerated GUI using OpenGL on Wayland.

## System Architecture

### Hardware Platform
- **Device**: NXP NavQ Plus
- **SoC**: i.MX8M Plus
- **GPU**: GC7000UL
- **Display**: 1280x720 resolution
- **Display Server**: Wayland

### Frontend (Qt/QML)
#### Technology Stack
- Qt 6.x
- QML with Material Design
- OpenGL ES 2.0/3.0
- Hardware acceleration via Wayland

#### Components
1. **Main Window** (`main.qml`)
   - Resolution: 1280x720
   - Material Dark theme
   - Green accent color

2. **Navigation Sidebar**
   - Dashboard view
   - Vehicle Status view
   - Settings view

3. **Data Visualization Components**
   - Real-time charts (QtCharts)
   - Vehicle status indicators
   - Warning/error displays

4. **Network Layer**
   - REST API client
   - Periodic data polling
   - Error handling and retry logic

### Backend (Flask)
#### Technology Stack
- Python 3.x
- Flask
- CAN bus interface

#### Components
1. **CAN Message Buffer**
   - In-memory storage for latest CAN messages
   - Message timestamp tracking
   - Data staleness detection

2. **API Endpoints**
```python
GET /api/v1/can/latest
    Returns: {
        "timestamp": int,  # Unix timestamp in ms
        "messages": {
            "speed": float,
            "battery_voltage": float,
            "motor_temp": float,
            # ... other CAN messages
        }
    }

GET /api/v1/can/status
    Returns: {
        "connected": bool,
        "uptime": int,  # seconds
        "message_rate": float  # messages/second
    }
```

3. **CAN Interface**
   - Message filtering
   - Timestamp tracking
   - Buffer management

## Data Flow
1. CAN messages arrive at backend
2. Messages are filtered and stored in memory buffer
3. Frontend polls backend API periodically
4. Data is displayed through Qt/QML components
5. Stale data detection triggers warnings

## Performance Requirements
- Frontend refresh rate: 60 FPS
- API polling interval: 100ms
- Maximum data staleness: 500ms
- GUI latency: <16ms (60 FPS)

## Security Considerations
1. **API Security**
   - Local network only
   - Basic authentication
   - Rate limiting

2. **Data Validation**
   - CAN message validation
   - API response validation
   - Bounds checking

## Error Handling
1. **Frontend**
   - Network error recovery
   - Data validation
   - Visual error indicators

2. **Backend**
   - CAN bus disconnection handling
   - Invalid message filtering
   - API error responses

## Development Setup
1. **Frontend Requirements**
```bash
sudo apt-get install qt6-base-dev qt6-declarative-dev
sudo apt-get install qt6-charts-dev
sudo apt-get install qt6-wayland
```

2. **Backend Requirements**
```txt
flask>=2.0.0
python-can>=4.0.0
```

## Detailed API Specifications

### REST API Endpoints

#### 1. CAN Data Endpoints

```python
GET /api/v1/can/latest
    Description: Get latest values for all CAN messages
    Response: {
        "timestamp": int,
        "messages": {
            "vehicle_speed": {
                "value": float,
                "unit": "km/h",
                "timestamp": int,
                "is_stale": bool
            },
            "battery_voltage": {
                "value": float,
                "unit": "V",
                "timestamp": int,
                "is_stale": bool
            },
            # ... other messages
        }
    }

GET /api/v1/can/message/<message_id>
    Description: Get specific CAN message
    Parameters: message_id (hex string)
    Response: {
        "value": float,
        "unit": string,
        "timestamp": int,
        "is_stale": bool
    }

GET /api/v1/can/status
    Description: Get CAN bus status
    Response: {
        "connected": bool,
        "uptime": int,
        "message_rate": float,
        "error_count": int
    }
```

#### 2. System Endpoints

```python
GET /api/v1/system/status
    Description: Get system status
    Response: {
        "cpu_temp": float,
        "gpu_temp": float,
        "memory_usage": float,
        "disk_usage": float
    }
```

### API Implementation (Flask)

```python
# app.py
from flask import Flask, jsonify
from datetime import datetime, timedelta

app = Flask(__name__)

class CANBuffer:
    def __init__(self):
        self.messages = {}
        self.stale_threshold = timedelta(milliseconds=500)
    
    def update_message(self, message_id, value, unit):
        self.messages[message_id] = {
            "value": value,
            "unit": unit,
            "timestamp": datetime.now()
        }
    
    def get_message(self, message_id):
        if message_id not in self.messages:
            return None
        
        msg = self.messages[message_id]
        msg["is_stale"] = (datetime.now() - msg["timestamp"]) > self.stale_threshold
        return msg

can_buffer = CANBuffer()

@app.route('/api/v1/can/latest')
def get_latest():
    return jsonify({
        "timestamp": datetime.now().timestamp(),
        "messages": {
            mid: can_buffer.get_message(mid)
            for mid in can_buffer.messages
        }
    })
```

## GUI Component Specifications

### 1. Main Layout (`main.qml`)

```qml
ApplicationWindow {
    // Base window properties
    width: 1280
    height: 720
    
    // Material theme configuration
    Material.theme: Material.Dark
    Material.accent: Material.Green
    
    // Layout structure
    RowLayout {
        // Sidebar (200px width)
        NavigationSidebar {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
        }
        
        // Main content area
        StackLayout {
            currentIndex: navigation.currentIndex
            
            DashboardView {}
            VehicleStatusView {}
            SettingsView {}
        }
    }
    
    // Status bar
    footer: StatusBar {}
}
```

### 2. Dashboard View

```qml
// DashboardView.qml
Item {
    ColumnLayout {
        // Speed display
        SpeedGauge {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
        }
        
        // Critical metrics
        Row {
            Layout.fillWidth: true
            
            MetricCard {
                title: "Battery Voltage"
                value: dataModel.batteryVoltage
                unit: "V"
            }
            
            MetricCard {
                title: "Motor Temp"
                value: dataModel.motorTemp
                unit: "°C"
            }
        }
        
        // Charts
        ChartView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            LineSeries {
                name: "Speed History"
                // ... data binding
            }
        }
    }
}
```

### 3. Data Model Integration

```cpp
// datamodel.h
class DataModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(double vehicleSpeed READ vehicleSpeed NOTIFY vehicleSpeedChanged)
    Q_PROPERTY(double batteryVoltage READ batteryVoltage NOTIFY batteryVoltageChanged)
    // ... other properties

public:
    explicit DataModel(QObject *parent = nullptr);

signals:
    void vehicleSpeedChanged();
    void batteryVoltageChanged();

private slots:
    void updateData();

private:
    QTimer *updateTimer;
    NetworkManager *network;
};
```

## CAN Message Format Specifications

### 1. Message Types

```cpp
struct CANMessage {
    uint32_t id;          // CAN message ID
    uint8_t dlc;         // Data length code
    uint8_t data[8];     // Message data
    uint64_t timestamp;  // Microsecond timestamp
};

// Message ID Definitions
enum class MessageID : uint32_t {
    VEHICLE_SPEED     = 0x100,
    BATTERY_VOLTAGE   = 0x200,
    MOTOR_TEMP        = 0x300,
    MOTOR_RPM        = 0x400,
    BRAKE_PRESSURE   = 0x500,
    ACCELERATOR_POS  = 0x600
};
```

### 2. Message Formats

```cpp
// Speed message (ID: 0x100)
struct SpeedMessage {
    uint16_t speed_kph;    // Bytes 0-1: Speed in km/h * 100
    uint8_t direction;     // Byte 2: 0=Forward, 1=Reverse
    uint8_t valid;         // Byte 3: Data validity
};

// Battery message (ID: 0x200)
struct BatteryMessage {
    uint16_t voltage;      // Bytes 0-1: Voltage in V * 100
    uint16_t current;      // Bytes 2-3: Current in A * 100
    uint16_t temp;         // Bytes 4-5: Temperature in °C * 10
    uint8_t soc;          // Byte 6: State of charge (%)
};
```

## Development and Deployment Procedures

### 1. Development Environment Setup

```bash
# Frontend development setup
sudo apt-get update
sudo apt-get install -y \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-charts-dev \
    qt6-wayland \
    cmake \
    ninja-build

# Backend development setup
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

### 2. Build Process

```bash
# Frontend build
mkdir build && cd build
cmake .. -GNinja
ninja

# Backend build
python -m pip install --upgrade build
python -m build
```

### 3. Deployment

```bash
# Deploy frontend
./deploy_frontend.sh
#!/bin/bash
# deploy_frontend.sh
ssh navq "mkdir -p /opt/ecocar/hmi"
scp build/ecocar-hmi navq:/opt/ecocar/hmi/
scp -r assets/* navq:/opt/ecocar/hmi/

# Deploy backend
./deploy_backend.sh
#!/bin/bash
# deploy_backend.sh
ssh navq "mkdir -p /opt/ecocar/server"
scp -r server/* navq:/opt/ecocar/server/
ssh navq "cd /opt/ecocar/server && python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt"
```

### 4. System Service Setup

```ini
# /etc/systemd/system/ecocar-hmi.service
[Unit]
Description=EcoCar HMI Frontend
After=network.target

[Service]
Type=simple
User=ecocar
Environment=QT_QPA_PLATFORM=wayland
Environment=XDG_RUNTIME_DIR=/run/user/1000
ExecStart=/opt/ecocar/hmi/ecocar-hmi
Restart=always

[Install]
WantedBy=multi-user.target

# /etc/systemd/system/ecocar-server.service
[Unit]
Description=EcoCar Backend Server
After=network.target

[Service]
Type=simple
User=ecocar
WorkingDirectory=/opt/ecocar/server
ExecStart=/opt/ecocar/server/venv/bin/python app.py
Restart=always

[Install]
WantedBy=multi-user.target
```
