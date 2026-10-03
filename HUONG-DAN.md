# BOT FARMER X DLee — cập nhật v0.1.1

Tên cửa sổ/menu vẫn là **BOT FARMER X DLee v0.1**.

## Chạy chương trình

- Chạy `BOT-FARMER-X-DLee-v0.1.1.exe`: đã kèm DLL và ảnh, tự giải nén vào LocalAppData. Không cần cài đặt hoặc quyền quản trị.
- Nếu dùng ZIP: giải nén đầy đủ và chạy `autofarmnongtrai-update.exe`; giữ DLL và `images` cạnh EXE.
- Đặt khung game LDPlayer **960 × 540**, bấm **QUET TAT CA LDPLAYER**, chọn tab, chọn tùy chọn và danh sách trước khi **START BOT**. Bấm **STOP BOT** để dừng.

## Lưu lựa chọn

Lần đầu mở chưa có cấu hình: toàn bộ checkbox và danh sách trái/hạt/công cụ đều bỏ chọn. Bot không tự bật chức năng hoặc chọn sẵn mặt hàng.

Mỗi lần bạn đổi checkbox hoặc chế độ, chương trình tự lưu ngay; có thể bấm **LUU LUA CHON** để lưu thủ công. Lần sau mở hoặc quét lại LDPlayer sẽ khôi phục cấu hình theo tên tab. Cấu hình nằm ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings` và được giữ khi đổi bản EXE. Đổi tên tab LDPlayer sẽ dùng profile khác.

Lưu cả danh sách đã bỏ chọn hết; các tùy chọn được lưu gồm bán, thu hoạch, Tên Tím, mua hạt/công cụ, chế độ thu hoạch, Show MAT BOT và lựa chọn săn thời tiết/map. Khôi phục cấu hình không tự START BOT. Nếu không đọc/ghi được, menu báo lỗi rõ ràng.

## Chức năng giữ nguyên

- Thu hoạch trực tiếp qua **Thu hoạch**, không cần về **Nhà ta**.
- Bật **Thu hoach bang TEN TIM (dong loat)** nếu tài khoản có Tên Tím; bỏ chọn để thu hoạch thường.
- Bộ lọc thu hoạch bỏ qua trái không có, không cuộn tìm.
- Danh sách 29 loại hạt, mua hạt/công cụ theo chu kỳ.
- Auto BAN là tùy chọn; game có thể tự chọn nông sản cấp cao khi bán.
- Giữ bản sửa nhận diện nút shop hạt và luồng vào NPC.

## Kiểm tra

35 kiểm tra lưu/khôi phục cấu hình đã đạt, gồm profile riêng, bỏ chọn hết, tệp lỗi và lỗi ghi. Đã thử menu thật: mở đầu trống, thay lựa chọn, lưu thủ công, đóng bản thường rồi mở EXE đóng gói và khôi phục đúng; quét lại LDPlayer cũng khôi phục. Không thử lại các giao dịch mua/bán trong lần cập nhật này.

Mã nguồn, script build và giấy phép thư viện nằm trong `source`/`licenses`. `SHA256SUMS.txt` đối chiếu tệp ZIP; `.exe.sha256` đối chiếu EXE một file.
