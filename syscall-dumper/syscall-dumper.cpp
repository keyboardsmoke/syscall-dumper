#include "syscall-dumper.hpp"
#include <Windows.h>
#include <intrin.h>

typedef NTSTATUS(*NtInvocation_t)(PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID);

int Handler(EXCEPTION_POINTERS* Ep, uint32_t* syscallIndex)
{
    // 0F 05 C3 = syscall; ret
    // CD 2E C3 = int 2e; ret

    auto RipPointer = reinterpret_cast<uint8_t*>(Ep->ContextRecord->Rip);

    if (memcmp(RipPointer, "\x0f\x05\xc3", 3) == 0 ||
        memcmp(RipPointer, "\xcd\x2e\xc3", 3) == 0) {
        *syscallIndex = static_cast<uint32_t>(Ep->ContextRecord->Rax);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    // Trap Flag
    Ep->ContextRecord->EFlags |= 0x100;

    return EXCEPTION_SINGLE_STEP;
}

std::optional<uint32_t> GetSyscallIndex(void* SyscallCaller)
{
    uint32_t syscallIndex = 0;


    NtInvocation_t CallerFn = reinterpret_cast<NtInvocation_t>(SyscallCaller);

    __try {
        // Set Trap Flag (Single Step)
        __writeeflags(__readeflags() | 0x100);
        CallerFn(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    __except (Handler(GetExceptionInformation(), &syscallIndex)) {
        return syscallIndex;
    }

    return std::nullopt;
}