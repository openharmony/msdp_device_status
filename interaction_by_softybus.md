@startuml simple

box "DeviceA" #LightBlue
participant MSDP_A
end box

box "DeviceB" #LightPink
participant MSDP_B
end box

MSDP_A -> MSDP_B : SubscribeMouseLocation 

MSDP_B -> MSDP_B : OnSubscribeMouseLocation

MSDP_A -> MSDP_A : WaitForRelaySubscribeByBlock

MSDP_B -> MSDP_A : RelaySubscribeMouseLocation

MSDP_A -> MSDP_A : OnRelaySubscribeLocation

MSDP_A -> MSDP_A : CheckSubscribeStatus

MSDP_B -> MSDP_B : OnPointerEvent

MSDP_B -> MSDP_A : SyncMouseLocation

MSDP_A -> MSDP_A : OnMouseLocation

@enduml 
