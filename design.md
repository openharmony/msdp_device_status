@startuml simple

box "DeviceA" #LightBlue
participant 键鼠应用
participant MSDP_A
participant SoftBus_A
end box

box "DeviceB" #LightPink
participant SoftBus_B
participant MSDP_B
participant MMI_B
end box

键鼠应用 -> MSDP_A : RegisterLocationListener

MSDP_A -> SoftBus_A : SendBytes

SoftBus_A -> SoftBus_B: SendBytes

SoftBus_B -> MSDP_B : OnBytes(对端收到注册请求)

MSDP_B -> MSDP_B : 执行添加监听的逻辑

MSDP_B -> SoftBus_B : 回执A端添加监听是否成功

SoftBus_B -> SoftBus_A : SendBytes

SoftBus_A -> MSDP_A : OnBytes

MSDP_A -> 键鼠应用 : 通知添加监听是否OK

MMI_B -> MSDP_B : OnPointerEvent

MSDP_B -> SoftBus_B : 向A同步当前鼠标位置信息

SoftBus_B -> SoftBus_A : SendBytes

SoftBus_A -> MSDP_A : OnBytes

MSDP_A -> 键鼠应用 : NotifyLocation


@enduml 
