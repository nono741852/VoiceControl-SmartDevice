# VoiceControl-SmartDevice
## 基于 i.MX6ULL 的边缘–云协同智能语音控制终端

VoiceControl-SmartDevice 是一套运行于 i.MX6ULL 嵌入式平台的边缘智能语音交互系统。

系统采用 “本地快速响应 + 云端大模型推理” 双路径控制架构，实现语音识别、环境感知、硬件控制与云端协同决策的统一集成。

本项目重点不在单一功能实现，而在于：

- 边缘计算与云端 AI 的协同架构设计
- MQTT 与 HTTP 双通信链路的融合
- 嵌入式多线程实时数据采集模型
- 结构化指令执行框架设计

适用于 AIoT、边缘计算与智能终端方向的工程实践

## 系统架构
系统采用三层协同架构：

- 边缘层：运行在 i.MX6ULL 平台上，负责本地语音采集与识别、本地规则匹配与快速控制、环境数据采集与硬件控制。
- 云端推理层：基于dify平台开发的语义解析agent对复杂指令进行解析，生成结构化指令。
- 物联网平台层：基于中国移动 OneNET Studio 平台，实现设备状态上报与远程控制指令接收。
### 系统逻辑流程
```bash
麦克风              OneNET平台
  ↓                  ↓
语音识别模块          MQTT
  ↓                  ↓   
指令分发器          命令执行器
 ├── 本地控制路径 ─────┘
 │      ├── LED
 │      ├── 蜂鸣器
 │      └── 音乐播放器
 │
 └── 云端AI路径
        ↓
    HTTP请求 → Dify
        ↓
    返回结构化JSON指令
        ↓
    命令执行器

同时：
传感器线程 → MQTT → OneNET平台
```


## 核心模块功能

### 语音识别模块
- 基于百度云语音识别API
- 支持中文语音命令识别
- 支持实时录音与结果解析
- 支持本地规则快速匹配

### 云端AI推理模块
- 基于dify平台开发的大模型语义解析agent
- 通过 HTTP RESTful API 接入 Dify
- 支持复杂指令解析（如顺序执行）
- 支持自定义指令扩展
- 输出标准结构化 JSON 控制指令

大模型节点prompt示例：
```
你是一个【嵌入式设备控制语义解析 Agent】。

你的职责：
- 接收用户的自然语言指令{{#context#}}
- 将指令解析为【结构化设备控制命令】
- 输出结果必须是【严格 JSON 格式】，用于嵌入式设备端自动解析与执行
-无法匹配任何已知指令，返回 error 类型 JSON

⚠️【重要规则 — 必须严格遵守】：
1. 只允许输出 JSON，不允许输出任何解释、注释、自然语言
2. JSON 结构必须完全符合下方“命令协议”，不要包含换行符或额外的格式化
3. 不允许缺失字段，不允许新增字段
4. 如果无法匹配任何已知指令，返回 error 类型 JSON
5. 所有字段值必须使用小写英文
6. params 必须是一个 JSON 对象，不可为 null（无参数时返回空对象 {}）

--------------------------------
【统一命令协议（必须严格遵守）】

{
  "type": "control | query | error",
  "target": "led | beep| music | sensor | unknown",
  "command": "on | off | alarm | play | pause | stop | next | prev | getdata | unknown",
  "params": {}
}

--------------------------------
【已支持的语义能力】

1️⃣ 硬件控制：
- “开灯” → led + on
- “关灯” → led + off
- “报警” / “鸣笛” → beep+ alarm

2️⃣ 音乐控制：
- “播放音乐 / 播放歌曲” → music + play
- “暂停音乐 / 暂停歌曲” → music + pause
- “停止音乐 / 停止歌曲” → music + stop
- “下一首 / 下一曲” → music + next
- “上一首 / 上一曲” → music + prev

3️⃣ 环境查询：
- “环境数据 / 光照强度如何 / 传感器数据” → sensor + getdata
--------------------------------
【错误处理规则返回error 类型 JSON】
当用户输入无法匹配任何上述已支持命令时，必须返回：

{  "type": "error",  "target": "unknown",  "command": "unknown",  "params": {}}
--------------------------------
【示例（仅用于理解，不可原样输出）】
用户：“下一首”
正确输出：{  "type": "control",  "target": "music",  "command": "next",  "params": {}}
用户: 关灯
期望响应: {"type": "control", "target": "led", "command": "off", "params": {}}
--------------------------------



【再次强调】

- 不要推理过程
- 不要解释
- 不要输出多余文本
- 只输出 JSON，且不要包含换行符
```
### MQTT物联网通信模块
- 基于 MQTT 协议接入中国移动 OneNET Studio 平台
- 实现设备属性上报（post），支持手机 App 实时查看。
- 实现远程控制指令下发（set）
- 支持回执机制（set_reply / post_reply），支持自动重连机制

通信链路定位：

- MQTT：设备状态与远程控制
- HTTP：云端语音识别与语义推理

### 多线程传感器采集模块
- 采用多线程架构，确保传感器数据采集与 GUI 响应互不干扰。
- 支持 AP3216C 环境传感器数据采集（光照强度、接近距离、红外数据）
- 数据采集周期可配置，默认1秒采集一次。
  
核心逻辑
```cpp
void SensorThread::run() {
    while (m_isRunning) {
        m_sensor->updateSensorData();
        emit sensorDataUpdated();
        msleep(m_updateInterval);
    }
}
```

### 命令执行器模块

- 采用统一结构化执行模型：

  - 输入：JSON 控制指令
  - 分发：CommandExecutor
  - 执行：具体硬件模块
- 命令执行结果通过 MQTT 上报到 OneNET 平台
- 控制逻辑与语音模块解耦
- 支持自定义指令扩展（如添加新的设备控制）

### 用户界面模块
- 现代化简洁UI设计
- 语音识别状态实时反馈
- 传感器数据可视化显示
- 音乐播放状态和当前曲目显示

## 硬件平台

- 基于正点原子i.mx6ull alpha开发板（支持Linux/Qt运行环境）
- AP3216C环境传感器
- LED灯（或其他可控制硬件）
- 音频输入设备（麦克风）
- 音频输出设备（扬声器或耳机）

## 软件依赖


  | 项目           | 版本                  |
| ------------ | ------------------- |
| Linux Kernel | 建议 4.x              |
| Qt           | 5.12.9（测试版本）        |
| 编译工具链        | arm-linux-gnueabihf |
| GCC          | 4.8+                |
| MQTT         | Qt MQTT / Paho      |
| 云端AI         | Dify API            |


## 项目结构
- `02_asr_demo/`：主应用程序代码    
- -`mainwindow.h`：主窗口头文件
- -`mainwindow.cpp`：主窗口代码
- `asr/`：语音识别模块代码
- `audiorecorder/`：音频录制模块代码
- `led/`：LED控制模块代码
- `beep/`：蜂鸣器控制模块代码
- `sensor/`：传感器数据采集代码
- `musicplayer/`：音频播放控制代码
- `mqttclient/`：MQTT客户端通信代码
- `commandexecutor/`：命令执行器模块代码
- `cloudagent/`：云端Agent通信代码
- `myMusic/`：音乐文件存储目录
  


## 构建与部署

### 1. 克隆项目
```bash
git clone https://github.com/your-username/VoiceControl-SmartDevice.git
cd VoiceControl-SmartDevice
```

### 2. 配置API密钥
#### 配置百度云API密钥
在 `asr/asr.cpp` 文件中配置您的百度云API密钥：
```cpp
// 关键：替换为您的百度云API密钥！！！！
QString apiKey = "your_api_key";
QString secretKey = "your_secret_key";
```
#### 配置Dify API密钥
在 `cloudagent/cloudagent.h` 和 `mainwindow.cpp` 中配置您的Dify API参数：
```cpp
// 在mainwindow.cpp中设置Dify API地址和密钥
m_cloudAgent->setApiUrl("https://api.dify.ai/v1/chat-messages");  // 替换为您的Dify API地址
m_cloudAgent->setApiKey("your_dify_api_key");  // 替换为您的Dify API密钥
```

### 3. 配置OneNET云端参数
使用工具生成 设备级 Token（资源路径需包含 devices/device_name）
确保 OneNET 控制台定义的标识符（Identifier）与代码中的 als、ir、led 完全一致。
在 `mqttclient/mqttclient.h` 文件中配置您的OneNET设备参数：
```cpp
// 替换为您自己的OneNET设备参数
const QString m_productId = "your_product_id";
const QString m_deviceName = "your_device_name";
const QString m_token = "your_device_token";
```

### 4. 添加音乐文件
将WAV格式的音乐文件复制到 `myMusic/` 目录下。
### 5. 编译项目
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
  - **硬件控制**：
    - "开灯" - 控制LED灯开启
    - "关灯" - 控制LED灯关闭
    - "报警"/"鸣笛" - 触发蜂鸣器报警
  - **环境查询**：
    - "环境数据"/"光照强度如何"/"传感器数据" - 查询所有环境传感器数据
  - **音乐控制**：
    - "播放音乐"/"播放歌曲" - 开始播放音乐
    - "暂停音乐"/"暂停歌曲" - 暂停播放
    - "停止音乐"/"停止歌曲" - 停止播放
    - "下一首"/"下一曲" - 播放下一首
    - "上一首"/"上一曲" - 播放上一首


### 3. 云端远程控制
- 通过OneNET平台或手机APP远程控制设备
- 支持远程开关LED、蜂鸣器、音乐播放器
- 实时查看设备状态和传感器数据
- 设备状态变化会同步至云端

### 4. 查看环境数据
当识别到环境数据查询命令时，系统会：
- 自动启动传感器数据采集线程
- 在UI上显示实时环境数据（光照强度、接近距离、红外数据）
- 数据显示7秒后自动隐藏

## 运行流程
1、启动程序
2、语音输入与语音识别
3、本地规则匹配
4、若为复杂语义 → 请求云端 AI
5、返回结构化指令
6、执行硬件控制
7、同步状态至云端
## 技术特性总结
- 边缘–云协同架构设计
- 双链路通信模型（MQTT+HTTP）
- 多线程实时数据采集与处理
- 结构化命令执行框架
- 基于大模型的语义解析与指令生成
- 可扩展模块化设计


## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：[https://github.com/your-username/VoiceControl-SmartDevice](https://github.com/your-username/VoiceControl-SmartDevice)
- 邮箱：ryn18247501992@163.com

## 更新日志
### v2.1.0 (2026-02-15)
- 新增Dify云端AI Agent功能
- 实现自然语言理解与智能控制
- 支持结构化JSON控制指令解析
- 双重云端控制方案（MQTT + HTTP API）
- 优化云端响应错误处理机制

### v2.0.0 (2026-02-05)
- 新增MQTT云端远程控制功能
- 实现OneNET平台设备接入
- 支持远程设备控制
- 实现设备状态实时同步至云端
- 支持传感器数据定时上报至云端


### v1.1.0 (2026-01-30)
- 新增音频播放功能
- 支持WAV格式音乐文件播放
- 实现音乐播放状态和当前曲目显示


### v1.0.0 (2026-01-26)
- 初始版本发布
- 实现语音识别功能与硬件控制
- 实现多线程传感器数据采集
---

**Enjoy building your smart voice control system!** 🚀