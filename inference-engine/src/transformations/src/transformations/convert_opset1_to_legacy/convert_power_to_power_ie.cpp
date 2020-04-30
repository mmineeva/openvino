// Copyright (C) 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "transformations/convert_opset1_to_legacy/convert_power_to_power_ie.hpp"

#include <memory>
#include <vector>

#include <ngraph/opsets/opset1.hpp>

#include <ngraph_ops/power.hpp>
#include <transformations/utils/utils.hpp>
#include <ngraph/rt_info.hpp>

void ngraph::pass::ConvertPowerToPowerIE::convert_power() {
    auto input_0 = std::make_shared<pattern::op::Label>(element::f32, Shape{1});
    auto input_1 = std::make_shared<pattern::op::Label>(element::f32, Shape{1});
    auto power = std::make_shared<ngraph::opset1::Power>(input_0, input_1);


    ngraph::graph_rewrite_callback callback = [](pattern::Matcher& m) {
        auto power = std::dynamic_pointer_cast<ngraph::opset1::Power> (m.get_match_root());
        if (!power) {
            return false;
        }
        auto node = power->input(1).get_source_output().get_node_shared_ptr();
        if (auto const_node = std::dynamic_pointer_cast<ngraph::opset1::Constant>(node)) {
            float value(0);
            if (!ngraph::op::util::get_single_value(const_node, value)) {
                return false;
            }

            auto power_ie = std::make_shared<ngraph::op::PowerIE>(power->input(0).get_source_output(), value, 1, 0);
            power_ie->set_friendly_name(power->get_friendly_name());
            ngraph::copy_runtime_info(power, power_ie);
            ngraph::replace_node(power, power_ie);
            return true;
        }
        return false;
    };

    auto m = std::make_shared<ngraph::pattern::Matcher>(power, "ConvertPowerToPowerIE");
    this->add_matcher(m, callback, PassProperty::CHANGE_DYNAMIC_STATE);
}