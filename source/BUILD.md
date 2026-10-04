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

PlantTest kiểm tra bộ nhớ balo dùng chung cho nhiều hạt, nhớ hạt thiếu, cập nhật sau mua/dùng hết, đổi tab và không dùng dữ liệu quét chưa hoàn tất. SettingsTest xác nhận không lưu tồn kho vào cấu hình.

Kiểm tra lọc trái bằng ảnh độc lập, không thao tác game:

```powershell
./source/build.ps1 -OpenCvBuild C:/opencv/build -HarvestTest
./harvest_test.exe ./source/harvest-fixtures
```

HarvestTest kiểm tra tất cả 31 tên, tên có tiền tố, lỗi OCR, tên tương tự, ô dịch vị trí, tên nhiều dòng và ảnh lọc thực tế. Cần Windows OCR (đã có trên máy dùng bot).
