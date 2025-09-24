#pragma once

#include <optional>

std::optional<uint32_t> GetSyscallIndex(void* SyscallCaller);