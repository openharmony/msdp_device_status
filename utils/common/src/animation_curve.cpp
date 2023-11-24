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

#include "animation_curve.h"

#include "devicestatus_define.h"

static const OHOS::Rosen::RSAnimationTimingCurve EASE_CURVE =
    OHOS::Rosen::RSAnimationTimingCurve::CreateCubicCurve(0.25f, 0.1f, 0.25f, 1.0f);

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "AnimationCurve" };
} // namespace

std::unordered_map<std::string, AnimationCurve::CurveCreator> AnimationCurve::curveMap = {
    { "cubic-bezier", std::bind(&AnimationCurve::CreateCubicCurve, std::placeholders::_1) },
    { "spring", std::bind(&AnimationCurve::CreateSpringCurve, std::placeholders::_1) },
    { "interpolating-spring", std::bind(&AnimationCurve::CreateInterpolatingSpring, std::placeholders::_1) },
    { "responsive-spring-motion", std::bind(&AnimationCurve::CreateResponseSpring, std::placeholders::_1) },
    { "steps", std::bind(&AnimationCurve::CreateStepsCurve, std::placeholders::_1) }};

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateCurve(const std::string &curveName, const std::vector<float> &curve)
{
    if (curveMap.find(curveName) == curveMap.end() || curveMap[curveName] == nullptr) {
        FI_HILOGE("Unsupported curve type");
        return EASE_CURVE;
    }
    return curveMap[curveName](curve);
}

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateCubicCurve(const std::vector<float> &curve)
{
    if (curve.size() != 4) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    return OHOS::Rosen::RSAnimationTimingCurve::CreateCubicCurve(curve[0], curve[1], curve[2], curve[3]);
}

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateSpringCurve(const std::vector<float> &curve)
{
    if (curve.size() != 4) {
        FI_HILOGE("Invalid parameter");
        return EASE_CURVE;
    }
    return OHOS::Rosen::RSAnimationTimingCurve::CreateSpringCurve(curve[0], curve[1], curve[2], curve[3]);
}

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateInterpolatingSpring(const std::vector<float> &curve)
{
    if (curve.size() != 4) {
        FI_HILOGE("Invalid parameter");
        return EASE_CURVE;
    }
    return OHOS::Rosen::RSAnimationTimingCurve::CreateInterpolatingSpring(curve[0], curve[1], curve[2], curve[3]);
}

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateResponseSpring(const std::vector<float> &curve)
{
    if (curve.size() != 3) {
        FI_HILOGE("Invalid parameter");
        return EASE_CURVE;
    }
    return OHOS::Rosen::RSAnimationTimingCurve::CreateSpring(curve[0], curve[1], curve[2]);
}

OHOS::Rosen::RSAnimationTimingCurve AnimationCurve::CreateStepsCurve(const std::vector<float> &curve)
{
    if (curve.size() != 2) {
        FI_HILOGE("Invalid parameter");
        return EASE_CURVE;
    }
    auto steps = static_cast<int32_t>(curve[0]);
    auto stepPosition = static_cast<OHOS::Rosen::StepsCurvePosition>(static_cast<int32_t>(curve[1]));
    return OHOS::Rosen::RSAnimationTimingCurve::CreateStepsCurve(steps, stepPosition);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHO
