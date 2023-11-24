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

#ifndef ANIMATION_CURVE_H
#define ANIMATION_CURVE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "animation/rs_animation_timing_curve.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

class AnimationCurve {
public:
    using CurveCreator = std::function<Rosen::RSAnimationTimingCurve(const std::vector<float> &)>;
    static OHOS::Rosen::RSAnimationTimingCurve CreateCurve(const std::string &curveName, const std::vector<float> &curve);
private:
    static OHOS::Rosen::RSAnimationTimingCurve CreateCubicCurve(const std::vector<float> &curve);
    static OHOS::Rosen::RSAnimationTimingCurve CreateSpringCurve(const std::vector<float> &curve);
    static OHOS::Rosen::RSAnimationTimingCurve CreateInterpolatingSpring(const std::vector<float> &curve);
    static OHOS::Rosen::RSAnimationTimingCurve CreateResponseSpring(const std::vector<float> &curve);
    static OHOS::Rosen::RSAnimationTimingCurve CreateStepsCurve(const std::vector<float> &curve);
private:
    static std::unordered_map<std::string, CurveCreator> curveMap;
};

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // ANIMATION_CURVE_H