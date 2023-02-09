/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JS_DRAG_MANAGER_H
#define JS_DRAG_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "napi/native_node_api.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class JsDragManager {
public:
    JsDragManager();
    ~JsDragManager() = default;
    DISALLOW_COPY_AND_MOVE(JsDragManager);

    void RegisterListener(napi_env env, const std::string &type, napi_value handle);
    void UnregisterListener(napi_env env, const std::string &type, napi_value handle = nullptr);
    void ResetEnv();
    void RegisterThumbnailDraw(napi_env env, napi_value* argv);
    void UnregisterThumbnailDraw(napi_env env, napi_value argv);
    
private:
    struct CallbackInfo : public RefBase {
        napi_env env { nullptr };
        napi_ref ref { nullptr };
    };

    struct ThumbnailDrawCb : public RefBase {
        napi_env env { nullptr };
        napi_ref ref[3] { nullptr };
        int32_t errCode { -1 };
        napi_deferred deferred { nullptr };
        bool isApi9 { false };
    };
private:
    void ReleaseReference();
    bool IsSameHandle(napi_env env, napi_value handle, napi_ref ref);
    void EmitStartThumbnailDraw(int32_t pixmap);
    void EmitNoticeThumbnailDraw(int32_t dragState);
    void EmitEndThumbnailDraw();
    void EmitUnregisterThumbnailDraw(sptr<CallbackInfo> callbackInfo);
    
private:
    std::mutex mutex_;
    bool hasRegistered_ { false };
    inline static std::map<std::string, std::vector<std::unique_ptr<CallbackInfo>>> listeners_ {};
    sptr<ThumbnailDrawCb> thumbnailDrawCb_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // JS_DRAG_MANAGER_H
