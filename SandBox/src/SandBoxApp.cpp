#include <AyinPch.h>

#include <Ayin.h>

// ---入口点--------------------------
#include "Ayin/Core/EntryPoint.h"
// -----------------------------------

#include "Tests/SystemTestLayer.h"
#include "Tests/TestEnvironment.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

namespace {
	Ayin::Layer* CreateSystemTestLayer() {
		return new SystemTestLayer{};
	}

	struct TestLayerRegistration {
		std::string_view Name;
		Ayin::Layer* (*Create)();
	};

	constexpr std::array kTestLayers{
		TestLayerRegistration{ "system", &CreateSystemTestLayer },
	};
}

class SandBox : public Ayin::Application {
public:
	SandBox() {
		const std::string selectedLayer = SandBoxTests::NormalizeName(SandBoxTests::ReadEnvironmentVariable(SandBoxTests::LayerSelectionVariable));
		if (selectedLayer.empty() || selectedLayer == "none") {
			AYIN_INFO("No SandBox test layer selected. Set {0}=system to run System tests.", SandBoxTests::LayerSelectionVariable);
			if (SandBoxTests::IsTruthyEnvironmentVariable(SandBoxTests::AutoExitVariable))
				Close();
			return;
		}

		const auto registration = std::ranges::find_if(kTestLayers,
			[&selectedLayer](const TestLayerRegistration& candidate) {
				return candidate.Name == selectedLayer;
			});
		if (registration == kTestLayers.end()) {
			AYIN_ERROR("Unknown SandBox test layer '{0}' from {1}.", selectedLayer, SandBoxTests::LayerSelectionVariable);
			if (SandBoxTests::IsTruthyEnvironmentVariable(SandBoxTests::AutoExitVariable))
				Close();
			return;
		}

		AYIN_INFO("Starting SandBox test layer: {0}", selectedLayer);
		PushLayer(registration->Create());
	}

	~SandBox() override = default;
};

Ayin::Application* Ayin::CreatApplication() {
	return new SandBox();
}
