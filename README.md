# BOT FARMER X DLee

Auto nông trại cho LDPlayer, Windows 11/10 x64. Menu **BOT FARMER X DLee v0.1**, bản mới **v0.1.8**.

[Tải EXE v0.1.8](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/download/v0.1.8/BOT-FARMER-X-DLee-v0.1.8.exe) · [Bản phát hành](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/tag/v0.1.8)

- Làm lại trồng theo sơ đồ nhiều luống, đi từ gần cổng lên trên, đo chuyển động từng bước và giữ điểm đã kiểm tra/đã trồng. Chỉ trồng khi vòng xanh hợp lệ và kết quả được xác nhận.
- Quét toàn bộ hạt một lượt, nhớ tên/ảnh/số lượng/trang/vị trí. Đổi hạt dùng vị trí đã nhớ và xác nhận đúng gói; thiếu bỏ qua. Số lượng chưa rõ hiện `?`.
- Chọn riêng 34 hạt trồng. Auto mua chỉ theo danh sách mua đã chọn; mua thêm cập nhật kho rồi tiếp tục trồng.
- Có nút quét vườn, thử đường đi không trồng, trồng thử một cây, đặt lại tiến độ và xem kho hạt đã nhớ.
- Giữ thu hoạch Tên Tím/thu thường, lọc trái bị ẩn không cuộn, sửa OK sau bán và mua hạt/công cụ tùy chọn.
- Lần đầu không tự tick. Lưu/khôi phục lựa chọn theo tab LDPlayer; mở lại không tự START.

Chạy EXE một file đã kèm thư viện và ảnh, hoặc giải nén ZIP đầy đủ. Đặt vùng game 960 × 540, quét LDPlayer và chọn đúng tab. Lúc khảo sát đầu tiên hãy để cửa sổ bot/LDPlayer được chọn để chỉnh camera; các lượt dùng sơ đồ đã định vị. STOP nhả cần di chuyển.

## Kiểm tra v0.1.8

145 kiểm tra sơ đồ/di chuyển/bộ nhớ hạt, 55 kiểm tra balo thật, 136 kiểm tra nhận diện trồng và 40 kiểm tra cấu hình đã đạt.

Đã thử trực tiếp trên LDPlayer: khảo sát và đi đến đủ 8/8 luống của vườn hiện tại, không mua hay trồng trong lượt kiểm tra đường đi. Quét balo một lượt, nhớ 12 loại hạt; trồng thử 1 cây Bắp rồi đổi sang Cà rốt và trồng thêm 1 cây. Bộ đếm quét toàn bộ vẫn là 1. Bắp cập nhật 4 → 3, Cà rốt 15 → 14; hai loại được chọn nhưng thiếu (Nho, Dưa) được bỏ qua. Tiến độ giữ 2 cây đã xác nhận. Lựa chọn ban đầu được khôi phục và đối chiếu nguyên vẹn.

Không thử lại toàn bộ giao dịch mua/bán và thu hoạch trong lần cập nhật này. Nhận diện ranh giới cần luống nhìn thấy và đường đi tiếp cận được; kết quả trên vườn hiện tại không chứng minh mọi bố cục khác đều hoạt động.

Xem [hướng dẫn](HUONG-DAN.md), [phân tích luồng](PHAN-TICH-TRONG-BALO.md) và [build/kiểm tra](source/BUILD.md). Mã nguồn ở `source`, ảnh ở `images`, giấy phép thư viện ở `licenses`. Chưa cấp giấy phép riêng cho mã bot.
