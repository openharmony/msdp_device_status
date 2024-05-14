
# 修改穿越后不能中银切换的鼠标显隐藏的逻辑

## 多模提供接口
```cpp
/*
    input_device.h
*/
enum VirtualDeviceEvent {
    DEVICE_ADDED,
    DEVICE_REMOVED
};

/*
    input_manager.h
*/
void NotifyVirtualDeviceInfo(std::shared_ptr<MMI::InputDevice> deviceInfo, VirtualDeviceEvent event);
std::shared_ptr<MMI::InputDevice> deviceInfo;
inputManager.NotifyVirtualDeviceInfo(deviceInfo, event);
```

## 获取本端设备上是否存在键盘，鼠标
```cpp



```

## 增加一条消息，向对端通知设备上下线
```cpp


```

## 对端收到后 调用多模的接口，执行产生热插拔事件的逻辑
```cpp


```
