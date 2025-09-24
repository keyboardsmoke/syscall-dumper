#include "ntdll_apis.hpp"
#include "win32u_apis.hpp"
#include "syscall-dumper.hpp"

#include <Windows.h>

#include <fstream>
#include <iostream>
#include <map>

#include <argparse/argparse.hpp>

using ApiMap = std::map<std::string, std::optional<uint32_t>>;

template<size_t N>
static void AddSetToMap(const std::wstring& moduleName, ApiMap& apiMap, const std::array<std::string, N>& apiSet) {
    auto mod = LoadLibrary(moduleName.c_str());
    if (mod == nullptr)
        return;

    for (const auto& api : apiSet) {
        auto fp = GetProcAddress(mod, api.c_str());
        if (fp == nullptr)
            continue;

        apiMap[api] = GetSyscallIndex(fp);
    }
}

static void HandleApiMap(const ApiMap& apiMap, bool shouldMakeJson, bool shouldShowEmpty, std::optional<std::string> outputFile) {
    std::stringstream ss;

    if (shouldMakeJson) {
        ss << "{" << std::endl;

        size_t current = 0;
        size_t maximum = apiMap.size();
        for (const auto& api : apiMap) {
            if (api.second.has_value()) {
                ss << "\t\"" << api.first << "\": " << api.second.value();
            }
            else if (shouldShowEmpty) {
                ss << "\t\"" << api.first << "\": null";
            }

            // If it's not the last entry, we need a comma
            if (current != (maximum - 1)) {
                ss << ",";
            }
            ss << std::endl;
            ++current;
        }

        ss << "}";
    }
    else {
        for (const auto& api : apiMap) {
            if (api.second.has_value()) {
                ss << api.first << " = " << api.second.value() << std::endl;
            }
            else if (shouldShowEmpty) {
                ss << api.first << " = <none>" << std::endl;
            }
        }
    }

    if (outputFile.has_value()) {
        std::ofstream ofs(outputFile.value(), std::ios::out);
        ofs << ss.str();
        ofs.close();
    }
    else {
        std::cout << ss.str() << std::endl;
    }
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser parser("syscall-dumper");

    parser.add_argument("--output");
    parser.add_argument("--json").flag();
    parser.add_argument("--show_empty").flag();

    try {
        parser.parse_args(argc, argv);
    }
    catch (std::exception& e) {
        std::cout << parser << std::endl;
        return 1;
    }

    bool shouldMakeJson = parser.get<bool>("--json");
    bool shouldShowEmpty = parser.get<bool>("--show_empty");
    bool shouldPrintToOutput = parser.is_used("--output");

    std::optional<std::string> output = std::nullopt;
    if (shouldPrintToOutput) {
        output = parser.get<std::string>("--output");
    }

    ApiMap apiMap;

    // Setup API map
    AddSetToMap(L"ntdll.dll", apiMap, ntdll_apis);
    AddSetToMap(L"win32u.dll", apiMap, win32u_apis);

    // Print Output
    HandleApiMap(apiMap, shouldMakeJson, shouldShowEmpty, output);

    return 0;
}