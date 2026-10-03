# Build Windows x64

Dùng Developer PowerShell x64 của Visual Studio 2022, C++20, Windows SDK và OpenCV 4.12.0 chính thức.

```powershell
./source/build.ps1 -OpenCvBuild C:/opencv/build
./source/build.ps1 -OpenCvBuild C:/opencv/build -SettingsTest
./settings_test.exe ./settings-test-scratch
./source/build.ps1 -OpenCvBuild C:/opencv/build -PlantTest
./plant_test.exe ./source/test-fixtures ./images
```

SettingsTest chỉ đổi LOCALAPPDATA trong tiến trình kiểm tra; không thao tác game. PlantTest dùng ảnh balo, vòng xanh và đất đã loại thông tin tài khoản, cùng dịch chuyển camera có giá trị chuẩn. Không chạy bot hay mua/bán.
