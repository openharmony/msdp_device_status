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
namespace {
using RosenCurveType = OHOS::Rosen::RSAnimationTimingCurve;
}

class AnimationCurve {
public:
    using CurveCreator = std::function<RosenCurveType(const std::vector<float> &)>;
    static RosenCurveType CreateCurve(const std::string &curveName, const std::vector<float> &curve);
private:
    static RosenCurveType CreateCubicCurve(const std::vector<float> &curve);
    static RosenCurveType CreateSpringCurve(const std::vector<float> &curve);
    static RosenCurveType CreateInterpolatingSpring(const std::vector<float> &curve);
    static RosenCurveType CreateResponseSpring(const std::vector<float> &curve);
    static RosenCurveType CreateStepsCurve(const std::vector<float> &curve);
private:
    static std::unordered_map<std::string, RosenCurveType> specialCurveMap_;
    static std::unordered_map<std::string, CurveCreator> curveMap_;
};

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // ANIMATION_CURVE_H