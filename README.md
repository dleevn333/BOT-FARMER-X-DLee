# BOT FARMER X DLee

Auto nông trại cho LDPlayer, Windows 11/10 x64. Menu **BOT FARMER X DLee v0.1**, bản mới **v0.1.7**.

[Tải EXE v0.1.7](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/download/v0.1.7/BOT-FARMER-X-DLee-v0.1.7.exe) · [Bản phát hành](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/tag/v0.1.7)

- Sửa không nhận vòng trồng xanh nhạt khi có hiệu ứng thời tiết, tìm khoảng đất rộng hơn và nhận hạt cuối x1; ưu tiên trồng lúc START, hiện kết quả riêng.
- Sửa kẹt OK sau bán: nhận hộp thoại/OK mới, thử lại có giới hạn và chờ bảng bán hiện lại trước khi chạy tiếp.
- Sửa chọn trái thu hoạch: đọc từng ô, tên xuống dòng và lỗi OCR nhỏ; lấy vị trí ô thực tế, xác nhận dấu tick. Có nút thử bộ lọc và báo loại chưa chọn được.
- Quét balo một lượt, nhớ cả hạt có/thiếu cho mọi loại đã chọn; mua thêm cập nhật lại, dùng hết đánh dấu thiếu. Có nút quét lại và bộ đếm lượt quét.
- Đợi ảnh balo ổn định và nhận diện ảnh hạt nhỏ ở các mức phóng gần nhất, giữ ngưỡng xác nhận đúng loại.
- Auto trồng: chọn hạt riêng, lấy trong balo, thiếu thì bỏ qua; mua các hạt đã chọn rồi trồng. Chỉ bấm khi nhận vòng xanh hợp lệ và kiểm tra kết quả.
- Tự về vườn, đi từ cổng vào luống, tìm đất trong vùng nhìn thấy. Có nút thử một cây và kiểm tra di chuyển; không đảm bảo phủ toàn bộ vườn với bố cục khác.
- 34 loại hạt trồng: 29 hạt cửa hàng và 5 hạt balo hiện có.
- Giữ thu hoạch Tên Tím, thu hoạch thường, bỏ cuộn lọc trái bị ẩn, mua hạt/công cụ và bán tùy chọn.
- Lần đầu không tự tick; tự lưu/khôi phục theo tab LDPlayer, gồm cả danh sách trồng. Đọc cấu hình v0.1.1; không tự START.

Chạy EXE một file đã kèm thư viện và ảnh, hoặc giải nén ZIP đầy đủ. Đặt vùng game 960 × 540, quét LDPlayer và chọn đúng tab trước khi START. STOP nhả cần di chuyển.

136 kiểm tra nhận diện/bộ nhớ hạt và 40 kiểm tra cấu hình đạt trong v0.1.7. 15 kiểm tra bán/OK của bản trước đã đạt; đã thử đóng hộp thoại đang kẹt và chạy luồng bán tự động đến khi trở về game trên LDPlayer.

Xem [hướng dẫn](HUONG-DAN.md) và [build/kiểm tra](source/BUILD.md). 153 kiểm tra lọc trái và 40 kiểm tra cấu hình đã đạt; giữ 130 kiểm tra nhận diện/bộ nhớ balo của v0.1.4. Đã thử chọn nhiều trái trên LDPlayer và đổi nhiều loại hạt với cùng một lượt quét; Thu riêng Nho bằng Tên Tím đã thành công và các loại không chọn giữ nguyên số lượng. Luồng trồng thật đã kiểm tra ở v0.1.2. Không thử lại toàn bộ giao dịch mua/bán trong lần cập nhật này.

Mã nguồn ở `source`, ảnh ở `images`, giấy phép thư viện ở `licenses`. Chưa cấp giấy phép riêng cho mã bot.
