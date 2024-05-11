# 错误码细化操作步骤

# 当前使用的错误码
```cpp
enum class CoordinationMessage {
    PREPARE = 0,
    UNPREPARE = 1,
    ACTIVATE = 2,
    ACTIVATE_SUCCESS = 3,
    ACTIVATE_FAIL = 4,
    DEACTIVATE_SUCCESS = 5,
    DEACTIVATE_FAIL = 6,
    SESSION_CLOSED = 7,
    COORDINATION_SUCCESS = 8
};

```

> 1. 先上库 失败后不上报任何错误码的修改，打通通路
> 2. 提接口,确定错误类型，保证错误码能够覆盖到相关领域的业务报错
> 3. 相关领域 DP 软总线 多模输入 DM 
> 4. 写DP失败 读DP失败 软总线建链失败 多模加监听失败  多模去监听失败  多模注入失败 DM或缺设备networkId失败