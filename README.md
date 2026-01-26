# VoiceControl-SmartDevice

基于Qt的语音识别智能设备控制系统，集成语音交互、硬件控制和环境感知功能。

## 功能特点

### 🎤 语音交互
- 基于百度云语音识别API的实时语音识别
- 支持中文语音命令识别
- 直观的语音交互界面（GIF动画反馈）

### 💡 硬件控制
- LED灯开关控制
- 可扩展支持多种硬件设备

### 🌡️ 环境感知
- AP3216C传感器数据采集（光照强度、接近距离、红外数据）
- 多线程数据采集，确保UI流畅度
- 实时环境数据显示

### 📱 用户界面
- 现代化简洁UI设计
- 语音识别状态实时反馈
- 传感器数据可视化显示

## 硬件要求

- 基于正点原子i.mx6ull alpha开发板（支持Linux/Qt运行环境）
- AP3216C环境传感器
- LED灯（或其他可控制硬件）
- 音频输入设备（麦克风）

## 软件依赖

- Qt 5.12+ (支持Widgets模块)
- GCC/G++编译器
- 百度云语音识别API密钥
- Linux系统（支持sysfs硬件访问）

## 项目结构
- `02_asr_demo/`：主应用程序代码    
- -`mainwindow.h`：主窗口头文件
- -`mainwindow.ui`：主窗口UI设计文件
- `beep/`：蜂鸣器控制代码
- `led/`：LED灯控制代码
- `sensor/`：传感器数据采集代码
- `mainwindow.cpp`：主窗口代码


## 安装与编译

### 1. 克隆项目
```bash
git clone https://github.com/your-username/VoiceControl-SmartDevice.git
cd VoiceControl-SmartDevice
```

### 2. 配置百度云API密钥
在 `asr/asr.cpp` 文件中配置您的百度云API密钥：
```cpp
// 替换为您的百度云API密钥
QString apiKey = "your_api_key";
QString secretKey = "your_secret_key";
```

### 3. 编译项目
使用Qt Creator打开 `02_asr_demo/02_asr_demo.pro` 文件，然后点击"构建"按钮。

或者使用命令行编译：
```bash
cd 02_asr_demo
qmake
make
```

## 使用说明

### 1. 启动应用
运行编译生成的可执行文件：
```bash
./02_asr_demo
```
在正点原子 I.MX6U 开发板上运行此录音程序，需要先配置是麦克风（板子上的麦头）。麦头录音，则在板子上运行开启麦头录音的脚本：/home/root/shell/audio/mic_in_config.sh
### 2. 语音交互
- 点击屏幕中央的GIF动画开始说话
- 等待"正在听您说话"提示
- 说出语音命令，如：
  - "开灯" - 控制LED灯开启
  - "关灯" - 控制LED灯关闭
  - “报警”/“鸣笛” - 触发蜂鸣器报警
  - "环境数据" /"光照强度如何"/“传感器数据”- 查询所有环境传感器数据

### 3. 查看环境数据
当识别到环境数据查询命令时，系统会：
- 自动启动传感器数据采集线程
- 在UI上显示实时环境数据（光照强度、接近距离、红外数据）
- 数据显示5秒后自动隐藏

## 核心功能实现

### 语音识别流程
1. 用户点击界面启动录音
2. 音频录制模块捕获音频数据
3. 语音识别模块将音频发送到百度云API
4. 接收并解析识别结果
5. 根据识别结果执行相应操作（硬件控制或数据查询）

### 多线程环境数据采集
```cpp
// SensorThread::run() 核心实现
void SensorThread::run() {
    while (m_isRunning) {
        // 采集传感器数据
        m_sensor->updateSensorData();
        
        // 获取并存储最新数据
        m_alsData = m_sensor->alsData();
        m_psData = m_sensor->psData();
        m_irData = m_sensor->irData();
        
        // 发送数据更新信号
        emit sensorDataUpdated();
        
        // 控制采集频率
        msleep(m_updateInterval);
    }
}
```

### 模块化设计
项目采用模块化架构，每个功能模块独立封装：
- **asr模块**：语音识别功能
- **audiorecorder模块**：音频录制功能  
- **led模块**：LED控制功能
- **sensor模块**：环境传感器功能

## 扩展指南

### 添加新的硬件控制
1. 创建新的硬件控制模块（参考led模块结构）
2. 实现硬件控制类
3. 在MainWindow中集成新模块
4. 在onAsrReadyData()中添加语音命令处理逻辑

### 支持新的传感器
1. 在sensor模块中添加新传感器驱动
2. 扩展SensorThread支持新传感器数据采集
3. 更新UI显示逻辑

## 项目依赖

| 模块     | 依赖项   | 版本要求 |
|---------|---------|----------|
| 核心框架 | Qt       | 5.12+ |
| 语音识别 | 百度云API | - |
| 硬件访问 | Linux sysfs | - |
| 编译工具 | GCC/G++   | 4.8+ |

## 许可证

本项目采用 MIT 许可证，详情请查看 LICENSE 文件。

## 贡献指南

欢迎提交 Issue 和 Pull Request！

### 提交代码前请确保：
- 代码符合项目的编码风格
- 添加了适当的注释
- 通过了基本的功能测试
- 更新了相关文档（如有必要）

## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：[https://github.com/your-username/VoiceControl-SmartDevice](https://github.com/your-username/VoiceControl-SmartDevice)
- 邮箱：your-email@example.com

## 更新日志

### v1.0.0 (2026-01-26)
- 初始版本发布
- 实现语音识别功能
- 支持LED灯控制
- 集成AP3216C环境传感器
- 实现多线程数据采集

---

**Enjoy building your smart voice control system!** 🚀