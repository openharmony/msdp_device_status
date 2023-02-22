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
#include "pixel_map.h"
#include "refbase.h"
#include <uv.h>

#include "i_drag_listener.h"
#include "util_napi_error.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class JsDragManager : public IDragListener, public std::enable_shared_from_this<JsDragManager> {
public:
    JsDragManager() = default;
    ~JsDragManager() = default;
    DISALLOW_COPY_AND_MOVE(JsDragManager);

    void ResetEnv();
    void OnDragMessage(DragMessage msg) override;
    void RegisterListener(napi_env env, napi_value handle);
    void UnregisterListener(napi_env env, napi_value handle = nullptr);
    void RegisterThumbnailDraw(napi_env env, size_t argc, napi_value* argv);
    void UnregisterThumbnailDraw(napi_env env, napi_value argv);

private:
    struct CallbackInfo : public RefBase {
        napi_env env { nullptr };
        napi_ref ref { nullptr };
        DragMessage msg;
    };

    struct ThumbnailDrawCb : public RefBase {
        napi_env env { nullptr };
        napi_ref ref[3] { nullptr };
        int32_t errCode { -1 };
        napi_deferred deferred { nullptr };
        int32_t pid { -1 };
        int32_t msgType { -1 };
        bool allowDragIn { false };
        std::u16string message;
        std::shared_ptr<DragData> dragData { nullptr };
    };

private:
    static void CallDragMsg(uv_work_t *work, int32_t status);
    void DeleteCallbackInfo(std::unique_ptr<CallbackInfo> callback);
    void ReleaseReference();
    bool IsSameHandle(napi_env env, napi_value handle, napi_ref ref);
    void EmitStartThumbnailDraw(std::shared_ptr<DragData> dragData);
    void EmitNoticeThumbnailDraw(int32_t msgType, bool allowDragIn, std::u16string message);
    void EmitEndThumbnailDraw(int32_t pid, int32_t result);
    void EmitUnregisterThumbnailDraw(sptr<ThumbnailDrawCb> callbackInfo);
    static void CallStartThumbnailDrawAsyncWork(uv_work_t *work, int32_t status);
    static void CallNoticeThumbnailDrawAsyncWork(uv_work_t *work, int32_t status);
    static void CallEndThumbnailDrawAsyncWork(uv_work_t *work, int32_t status);
    static void CallUnregisterThumbnailDrawAsyncWork(uv_work_t *work, int32_t status);
    static int32_t GetErrorCode(napi_env env, int32_t errCode, napi_value* callResult);
    static napi_value GetDragOption(sptr<ThumbnailDrawCb> cb);
    static napi_value GetNoticeMsg(sptr<ThumbnailDrawCb> cb);
    static napi_value GreateBusinessError(napi_env env, int32_t errCode, std::string errMessage);
    template <typename T>
    static void DeletePtr(T &ptr)
    {
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
    }
private:
    std::atomic_bool hasRegistered_ { false };
    sptr<ThumbnailDrawCb> thumbnailDrawCb_ { nullptr };
    inline static std::mutex mutex_;
    inline static std::vector<std::unique_ptr<CallbackInfo>> listeners_ {};
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // JS_DRAG_MANAGER_H
