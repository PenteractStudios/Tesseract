#pragma once

#include "Logging.h"

#include "AL/al.h"
#include <string>

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

bool check_al_errors(const std::string& filename, const std::uint_fast32_t line) {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		std::string errorText;
		switch (error) {
		case AL_INVALID_NAME:
			errorText = "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
			break;
		case AL_INVALID_ENUM:
			errorText = "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
			break;
		case AL_INVALID_VALUE:
			errorText = "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
			break;
		case AL_INVALID_OPERATION:
			errorText = "AL_INVALID_OPERATION: the requested operation is not valid";
			break;
		case AL_OUT_OF_MEMORY:
			errorText = "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
			break;
		default:
			errorText = "UNKNOWN AL ERROR: " + error;
		}
		LOG((filename.substr(filename.rfind("\\") + 1) + " (%d): " + errorText).c_str(), line);
		return false;
	}
	return true;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
	-> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))> {
	auto ret = function(std::forward<Params>(params)...);
	check_al_errors(filename, line);
	return ret;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
	-> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
	function(std::forward<Params>(params)...);
	return check_al_errors(filename, line);
}