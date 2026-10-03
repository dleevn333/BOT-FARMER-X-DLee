#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include "payload.h"

namespace fs = std::filesystem;
static void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
static bool matches(const fs::path& path, const char* bytes, DWORD length) {
    std::error_code ec;
    if (fs::file_size(path, ec) != length || ec) return false;
    std::ifstream in(path, std::ios::binary);
    std::vector<char> buffer(65536);
    for (DWORD offset = 0; offset < length;) {
        DWORD size = (std::min)(DWORD(buffer.size()), length - offset);
        in.read(buffer.data(), size);
        if (!in || std::memcmp(buffer.data(), bytes + offset, size) != 0) return false;
        offset += size;
    }
    return true;
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    HANDLE mutex = nullptr;
    bool locked = false;
    try {
        PWSTR local = nullptr;
        require(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local)), "Cannot locate LocalAppData.");
        fs::path root = fs::path(local) / L"BOT FARMER X DLee" / kPayloadVersion;
        CoTaskMemFree(local);
        mutex = CreateMutexW(nullptr, FALSE, kPayloadMutex);
        require(mutex != nullptr, "Cannot initialize package lock.");
        DWORD wait = WaitForSingleObject(mutex, 30000);
        require(wait == WAIT_OBJECT_0 || wait == WAIT_ABANDONED, "Another copy is extracting; try again shortly.");
        locked = true;
        for (const auto& item : kPayload) {
            HRSRC resource = FindResourceW(instance, MAKEINTRESOURCEW(item.id), RT_RCDATA);
            require(resource != nullptr, "Missing embedded resource.");
            DWORD size = SizeofResource(instance, resource);
            const char* bytes = static_cast<const char*>(LockResource(LoadResource(instance, resource)));
            require(bytes != nullptr && size > 0, "Invalid embedded resource.");
            fs::path destination = root / item.path;
            fs::create_directories(destination.parent_path());
            if (matches(destination, bytes, size)) continue;
            fs::path temporary = destination;
            temporary += L".new";
            {
                std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
                require(bool(out), "Cannot write application files.");
                out.write(bytes, size);
                out.flush();
                require(bool(out), "Cannot finish writing application files.");
            }
            require(MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH), "Cannot update application files. Close the running bot first.");
        }
        ReleaseMutex(mutex);
        locked = false;
        CloseHandle(mutex);
        mutex = nullptr;
        fs::path executable = root / L"autofarmnongtrai-update.exe";
        std::wstring command = L"\"" + executable.wstring() + L"\"";
        STARTUPINFOW startup{sizeof(startup)};
        PROCESS_INFORMATION process{};
        require(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, root.c_str(), &startup, &process), "Cannot start BOT FARMER X DLee.");
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return 0;
    } catch (const std::exception& error) {
        if (locked) ReleaseMutex(mutex);
        if (mutex) CloseHandle(mutex);
        MessageBoxA(nullptr, error.what(), "BOT FARMER X DLee v0.1", MB_OK | MB_ICONERROR);
        return 1;
    }
}
