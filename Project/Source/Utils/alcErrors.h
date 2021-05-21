#pragma once

#include "Logging.h"

#include "AL/alc.h"
#include <string>

#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device) {
	ALCenum error = alcGetError(device);
	if (error != ALC_NO_ERROR) {
		std::string errorText;
		switch (error) {
		case ALC_INVALID_VALUE:
			errorText = "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
			break;
		case ALC_INVALID_DEVICE:
			errorText = "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
			break;
		case ALC_INVALID_CONTEXT:
			errorText = "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
			break;
		case ALC_INVALID_ENUM:
			errorText = "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
			break;
		case ALC_OUT_OF_MEMORY:
			errorText = "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
			break;
		default:
			errorText = "UNKNOWN ALC ERROR: " + error;
		}
		LOG((filename.substr(filename.rfind("\\") + 1) + " (%d): " + errorText).c_str(), line);
		return false;
	}
	return true;
}

template<typename alcFunction, typename... Params>
auto alcCallImpl(const char* filename, const std::uint_fast32_t line, alcFunction function, ALCdevice* device, Params... params)
	-> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
	function(std::forward<Params>(params)...);
	return check_alc_errors(filename, line, device);
}

template<typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char* filename, const std::uint_fast32_t line, alcFunction function, ReturnType& returnValue, ALCdevice* device, Params... params)
	-> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool> {
	returnValue = function(std::forward<Params>(params)...);
	return check_alc_errors(filename, line, device);
}
