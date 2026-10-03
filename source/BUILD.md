# Build từ mã nguồn

Dùng Visual Studio 2022 Build Tools có C++, Windows SDK, OpenCV 4.12.0 x64 (thư mục `build` có `include` và `x64/vc16`). Mở **x64 Developer PowerShell**, chuyển đến `source` và chạy:

```powershell
.\build.ps1 -OpenCvBuild 'D:\opencv\build'
```

Giữ thư mục `images` và DLL OpenCV cạnh EXE. Script build dùng C++20, x64 Release và CRT tĩnh.

## EXE một file

Sau khi build bản thường, cài Python 3 và dùng cùng Developer PowerShell:

```powershell
.\single-exe\build-single.ps1 -ReleaseDirectory .. -OutputExe (Join-Path (Resolve-Path ..) 'BOT-FARMER-X-DLee-v0.1.1.exe') -ReleaseVersion v0.1.1
```

EXE chứa tài nguyên Win32, tự giải nén những tệp cần chạy rồi mở bot. Không tải thêm tệp qua mạng. Cấu hình người dùng nằm ngoài thư mục giải nén của từng phiên bản.

## Kiểm tra lưu cấu hình

```powershell
.\build.ps1 -OpenCvBuild 'D:\opencv\build' -SettingsTest
$testScratch = Join-Path $env:TEMP ('BotFarmerSettingsTest-' + [guid]::NewGuid().ToString('N'))
..\settings_test.exe $testScratch
```

Kiểm tra dùng thư mục riêng và chỉ đổi LOCALAPPDATA trong tiến trình test: mở lần đầu trống, lưu/khôi phục, tách profile, bỏ chọn hết, dữ liệu lỗi và lỗi ghi tệp. Không chạy bot hoặc thao tác game.
