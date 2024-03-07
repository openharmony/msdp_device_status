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

#undef LOG_TAG
#define LOG_TAG "AnimationCurve"

constexpr int32_t CUBIC_PARAM_LIMIT { 4 };
constexpr int32_t SPRING_PARAM_LIMIT { 4 };
constexpr int32_t INTERPOLATING_SPRING_PARAM_LIMIT { 4 };
constexpr int32_t RESPONSE_SPRING_PARAM_LIMIT { 3 };
constexpr int32_t STEPS_PARAM_LIMIT { 2 };

constexpr int32_t ARG_0 { 0 };
constexpr int32_t ARG_1 { 1 };
constexpr int32_t ARG_2 { 2 };
constexpr int32_t ARG_3 { 3 };

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
static const RosenCurveType EASE_CURVE = Rosen::RSAnimationTimingCurve::EASE;
} // namespace

std::unordered_map<std::string, RosenCurveType> AnimationCurve::specialCurveMap_ = {
    { "ease", Rosen::RSAnimationTimingCurve::EASE },
    { "ease-in", Rosen::RSAnimationTimingCurve::EASE_IN },
    { "ease-out", Rosen::RSAnimationTimingCurve::EASE_OUT },
    { "ease-in-out", Rosen::RSAnimationTimingCurve::EASE_IN_OUT },
    { "linear", Rosen::RSAnimationTimingCurve::LINEAR }
};

std::unordered_map<std::string, AnimationCurve::CurveCreator> AnimationCurve::curveMap_ = {
    { "cubic-bezier", std::bind(&AnimationCurve::CreateCubicCurve, std::placeholders::_1) },
    { "spring", std::bind(&AnimationCurve::CreateSpringCurve, std::placeholders::_1) },
    { "interpolating-spring", std::bind(&AnimationCurve::CreateInterpolatingSpring, std::placeholders::_1) },
    { "responsive-spring-motion", std::bind(&AnimationCurve::CreateResponseSpring, std::placeholders::_1) },
    { "steps", std::bind(&AnimationCurve::CreateStepsCurve, std::placeholders::_1) }
};

RosenCurveType AnimationCurve::CreateCurve(const std::string &curveName, const std::vector<float> &curve)
{
    if (specialCurveMap_.find(curveName) != specialCurveMap_.end()) {
        return specialCurveMap_[curveName];
    }
    if (curveMap_.find(curveName) == curveMap_.end() || curveMap_[curveName] == nullptr) {
        FI_HILOGE("Unknow curve type, use EASE");
        return EASE_CURVE;
    }
    return curveMap_[curveName](curve);
}

RosenCurveType AnimationCurve::CreateCubicCurve(const std::vector<float> &curve)
{
    if (curve.size() != CUBIC_PARAM_LIMIT) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    return RosenCurveType::CreateCubicCurve(curve[ARG_0], curve[ARG_1], curve[ARG_2], curve[ARG_3]);
}

RosenCurveType AnimationCurve::CreateSpringCurve(const std::vector<float> &curve)
{
    if (curve.size() != SPRING_PARAM_LIMIT) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    return RosenCurveType::CreateSpringCurve(curve[ARG_0], curve[ARG_1], curve[ARG_2], curve[ARG_3]);
}

RosenCurveType AnimationCurve::CreateInterpolatingSpring(const std::vector<float> &curve)
{
    if (curve.size() != INTERPOLATING_SPRING_PARAM_LIMIT) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    return RosenCurveType::CreateInterpolatingSpring(curve[ARG_0], curve[ARG_1], curve[ARG_2], curve[ARG_3]);
}

RosenCurveType AnimationCurve::CreateResponseSpring(const std::vector<float> &curve)
{
    if (curve.size() != RESPONSE_SPRING_PARAM_LIMIT) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    return RosenCurveType::CreateSpring(curve[ARG_0], curve[ARG_1], curve[ARG_2]);
}

RosenCurveType AnimationCurve::CreateStepsCurve(const std::vector<float> &curve)
{
    if (curve.size() != STEPS_PARAM_LIMIT) {
        FI_HILOGE("Invalid parameter, use EASE");
        return EASE_CURVE;
    }
    auto steps = static_cast<int32_t>(curve[ARG_0]);
    auto stepPosition = static_cast<OHOS::Rosen::StepsCurvePosition>(static_cast<int32_t>(curve[ARG_1]));
    return RosenCurveType::CreateStepsCurve(steps, stepPosition);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
