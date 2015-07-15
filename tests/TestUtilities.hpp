#pragma once

template<typename... T>
auto ElementsAreClose(T... v) -> decltype(ElementsAre(FloatNear(v, 0.00001f)...))
{
	return ElementsAre(FloatNear(v, 0.00001f)...);
}

