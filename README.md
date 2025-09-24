# syscall-dumper

Small Windows utility that enumerates `ntdll.dll` and `win32u.dll` exported functions and prints their syscall indices.

## How it works (high level)

For each known export name in `ntdll.dll` and `win32u.dll`, the tool:

- Dynamically resolves the function pointer with `GetProcAddress`
- Sets the CPU Trap Flag to single-step and invokes the stub with dummy args
- Catches the transition at `syscall; ret` (or `int 2e; ret`) via a structured exception handler (SEH) and reads RAX as the syscall index
- Bails upon instruction execution, so the fake call with fake arguments has no side-effect
- Returns the discovered index or marks it as missing if it cannot be obtained

## Usage

Basic:

```bash
syscall-dumper.exe
```

Flags:

- `--json`  Output JSON instead of text
- `--show_empty`  Include entries with no syscall number
- `--output <path>`  Write results to a file instead of stdout

Examples:

```bash
# Text to console (non-empty only)
syscall-dumper.exe

# JSON to console (non-empty only)
syscall-dumper.exe --json

# JSON including missing entries written to file
syscall-dumper.exe --json --show_empty --output syscalls.json
```

Sample text output:

```text
NtClose = 14
NtOpenProcess = 39
... 
```

Sample JSON output:

```json
{
  "NtClose": 14,
  "NtQuerySystemInformation": 186,
  "NtSetInformationProcess": null
}
```

Exit codes:

- `0` success
- `1` invalid arguments (usage shown)

## Limitations and notes

- Designed and tested for x64.
- Results depend on the current OS build; some entries may be absent or return null.
- Security products or debuggers may interfere with single-step/SEH behavior.
- The API name lists may lag behind newer Windows releases. Submit a PR!

## License

MIT. See [LICENSE](LICENSE)