# BOT FARMER X DLee

Auto nông trại cho LDPlayer trên Windows 11/10 x64. Tên menu: **BOT FARMER X DLee v0.1**. Bản cập nhật: **v0.1.1**.

[Tải EXE v0.1.1](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/download/v0.1.1/BOT-FARMER-X-DLee-v0.1.1.exe) · [Bản phát hành](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/tag/v0.1.1)

EXE đã kèm thư viện và ảnh, tự giải nén và mở bot, không cần quyền quản trị. Bản ZIP có chương trình, tài nguyên, hướng dẫn và mã nguồn.

## Sử dụng

Đặt khung game LDPlayer **960 × 540**, bấm **QUET TAT CA LDPLAYER**, chọn tab và các tùy chọn, rồi **START BOT**. **STOP BOT** để dừng.

Lần đầu mở không tự tick checkbox hoặc chọn sẵn trái/hạt/công cụ. Mọi thay đổi lựa chọn được tự lưu; nút **LUU LUA CHON** lưu thủ công. Lần sau mở hoặc quét lại LDPlayer khôi phục đúng profile theo tên tab. Cấu hình nằm ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings`, giữ được qua các bản EXE. Đổi tên tab LDPlayer sẽ dùng profile khác. Khôi phục cấu hình không tự chạy bot.

## Chức năng

- Thu hoạch trực tiếp qua **Thu hoạch**, không cần về **Nhà ta**.
- Thu hoạch đồng loạt bằng **Tên Tím**, tự nhận diện và xác nhận; bạn tự bật nếu tài khoản có Tên Tím.
- Bỏ cuộn tìm trái không có trong bộ lọc thu hoạch.
- Danh sách 29 loại hạt, mua hạt và công cụ theo chu kỳ.
- Bán tự động là tùy chọn; game có thể tự chọn nông sản cấp cao khi bán.
- Giữ bản sửa mở shop hạt trong cảnh sáng và bước đến đúng NPC.

## Mã nguồn và kiểm tra

Mã nguồn nằm trong `source`, tài nguyên trong `images`, giấy phép thư viện trong `licenses`. Xem [cách build và chạy kiểm tra](source/BUILD.md) và [hướng dẫn sử dụng](HUONG-DAN.md).

35 kiểm tra lưu/khôi phục đã đạt: mặc định trống, nhiều profile, bỏ chọn hết, dữ liệu lỗi, lỗi ghi và không khôi phục trạng thái đang chạy. Đã thử menu thật: đóng EXE thường rồi mở EXE đóng gói, khôi phục đúng các lựa chọn. Không thử lại các giao dịch mua/bán trong lần cập nhật này.

Chưa cấp giấy phép riêng cho mã bot; giấy phép thư viện đi kèm trong `licenses`.
