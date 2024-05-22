
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

# 两端的 input_device_mgr 中维护对端设备上的外设信息
> 首次穿越成功的时候将对端的外设信息同步过来，同步过来之后直接调用多模接口进行设置。
> 当热插拔之后，对端 input_device_mgr 中需要同步外设信息，外设信息同步之后直接调用多模接口注入热插拔事件。

## 获取本端设备上是否存在键盘，鼠标
> 获取本端设备上的键盘的id，以及相关信息
> 键盘相关信息的序列化、反序列化
> 当前框架中已经有维护外设信息的部分，需要在穿越的时候将键盘相关信息同步到对端
> 需要考虑将本端外设信息同步到对端的时机，借助 DP 实现是否具备可行性，貌似使用DP更为合理
> 组网成功之后同步设备信息
> 同步设备信息的时候使用和传图一致的逻辑，而不使用NetPacket,有大小限制
> 将外设信息序列化
> 必须和状态机绑定在一起
```cpp


```

## 增加一条消息，向对端通知设备上下线
> 当穿出端的键盘下线，需要通知到对端, 对端 input_device_mgr 中维护
> 可复用部分现有热插事件的逻辑，在其中加一些处理逻辑
```cpp


```

## 对端收到后 调用多模的接口，执行产生热插拔事件的逻辑
> 研究下如何在多模中新增一个接口
```cpp


```
