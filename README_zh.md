# MSDP设备状态感知框架<a name="ZH-CN_TOPIC_0000001148682248"></a>

-   [简介](#section_device_status_introduction)
-   [目录](#section_device_status_directory)
-   [约束](#section_device_status_constraint)
-   [使用](#section_device_status_usage)
    -   [接口说明](#section_device_status_interface)
    -   [使用说明](#section_device_status_sample)

-   [相关仓](#section_device_status_repository)

## 简介<a name="section_device_status_introduction"></a>

MSDP设备状态感知框架能够识别出目前设备的状态并传递给订阅者，整个框架是基于MSDP算法库和系统SensorHDI组件组成的，将其接收到的感知时间传递给订阅者。根据感知用途分为以下三大类：

-   绝对静止类：利用加速度、陀螺仪等传感器信息识别设备处于绝对静止状态。
-   水平/垂直姿态类：利用加速度、陀螺仪等传感器信息识别设备处于绝对静止状态。
-   皮套开合事件感知：基于霍尔传感器识别皮套的开合的状态。

MSDP设备状态感知架构图如下所示：

**图 1**  MSDP设备状态感知架构图  


![](figures/zh-cn_device_status_block.png)

## 目录<a name="section_device_status_directory"></a>

MSDP设备状态感知框架的示例代码如下：

```
/base/msdp/device_status
├── frameworks                 # 框架代码
│   └── native                 # device status客户端代码          
├── interfaces                 # 对外接口存放目录
│   ├── innerkits              # device satus innerkits API
│   └── kits/js                # Js API
├── sa_profile                 # 服务名称和服务的动态库的配置文件
├── services                   # 服务的代码目录
│   └── native/src             # device status服务代码
└── utils                      # 公共代码，包括权限、通信等能力
```

## 约束<a name="section_device_status_constraint"></a>

要使用MSDP设备状态感知功能，设备必须具有对应的传感器器件和相应的MSDP算法库。


## 使用<a name="section_device_status_usage"></a>

本节以MSDP设备状态感知框架JS API为例，说明其提供的具体功能以及使用流程。

###  接口说明<a name="section_device_status_interface"></a>

MSDP设备状态感知框架JS API: 同时监听Sensor HDI事件和MSDP算法库的设备状态事件，JS API开放的能力如下：

**表 1** JS API的主要接口

| 接口名      | 描述        |
| ----------- | ----------- |
|on(type: DeviceStatusType.TYPE_HIGH_STILL, callback: AsyncCallback<HighStillResponse>) |订阅设备状态的变化。type为支持订阅的设备状态类型，callback表示订阅设备状态类型的回调函数，TYPE_HIGH_STILL表示订阅类型|
|off(type: DeviceStatusType.TYPE_HIGH_STILL, callback: AsyncCallback<void>)|取消设备状态的订阅。type为支持的取消订阅的设备状态类型，callback表示取消订阅设备状态是否成功，TYPE_HIGH_STILL表示订阅类型|
|once(type: DevicestatusType.TYPE_HIGH_STILL, callback: AsyncCallback<HighStillResponse>)| 查询当前的设备状态。type为支持订阅的设备状态类型，callback表示订阅设备状态的回调函数，TYPE_HIGH_STILL表示订阅类型|

### 使用说明<a name="section_device_status_sample"></a>

1. 导包。
2. 注册并监听设备状态数据的变化。
3. 取消订阅设备状态数据的变化。
4. 注册并监听设备状态数据的变化一次。

示例代码:
```JavaScript
//步骤1 导包
import devicestatus from '@ohos.devicestatus';

export default {
    onCreate() {
        //步骤2 监听设备状态数据变化，并注册设备状态类型
        devicestatus.on(devicestatus.DevicestatusType.TYPE_HIGH_STILL, (error,data) => {
            if (error) {
                console.error("Failed to subscribe the device status type. Error code: " + error.code + "; message: " + error.message);
                return;
            }
            console.info("Device status value is： " + data.devicestatusValue);
            
        })
        //步骤3 设置10秒后取消设备状态数据
        setTimeout(function() {
            devicestatus.off(devicestatus.DevicestatusType.TYPE_HIGH_STILL, function(error) {
                if (error) {
                    console.error("Failed to unsubscribe from the device status type. Error code: " + error.code + "; message: " + error.message);
                    return;
                }
                console.info("Succeeded in unsubscribe from the device status type");
            });
        }, 10000);
        //步骤4 监听设备状态数据变化一次，并注册设备状态类型
        devicestatus.once(devicestatus.DevicestatusType.TYPE_HIGH_STILL, (error, data) => {
            if (error) {
                console.error("Failed to subscribe to  data. Error code: " + error.code + "; message: " + error.message);
                return;
            }
            console.info("Device status value is： " + data.devicestatusValue);
       });
    }

    onDestroy() {
        console.info('AceApplication onDestroy');
    }
}
```



## 相关仓<a name="section_device_status_repository"></a>

MSDP设备状态框架

**msdp\_device\_status**

