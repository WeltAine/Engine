#pragma once

#include "Ayin/Scene/SceneMode.h"
#include "Ayin/System/SystemTypes.h"

#include <glaze/glaze.hpp>


template<>
struct glz::meta<Ayin::SceneMode> {

	using enum Ayin::SceneMode;

	static constexpr auto value = glz::enumerate(
		None,

		Editor,
		Simulation,
		Runtime,

		AllSceneMode
	);

};


template<>
struct glz::meta<Ayin::SystemPhase> {

	using enum Ayin::SystemPhase;

	static constexpr auto value = glz::enumerate(
		None,

		PreUpdate,
		Update,
		PostUpdate,
		Presentation
	);

};
