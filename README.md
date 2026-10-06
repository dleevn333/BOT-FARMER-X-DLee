# BOT FARMER X DLee

Auto nông trại cho LDPlayer, Windows 11/10 x64. Menu **BOT FARMER X DLee v0.1**, bản mới **v0.1.10**.

[Tải EXE v0.1.10](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/download/v0.1.10/BOT-FARMER-X-DLee-v0.1.10.exe) · [Bản phát hành](https://github.com/dleevn333/BOT-FARMER-X-DLee/releases/tag/v0.1.10)

- Tự đóng OK của thông báo cây chưa đủ lớn và nhiệm vụ mới; chờ giao diện sẵn sàng trước khi tiếp tục. Lỗi cây chưa lớn có thời gian chờ 30 giây, không bấm lặp danh sách cũ.
- Làm lại trồng theo sơ đồ nhiều luống, đi từ gần cổng lên trên, đo chuyển động từng bước và giữ điểm đã kiểm tra/đã trồng. Chỉ trồng khi vòng xanh hợp lệ và kết quả được xác nhận.
- Quét toàn bộ hạt một lượt, nhớ tên/ảnh/số lượng/trang/vị trí. Đổi hạt dùng vị trí đã nhớ và xác nhận đúng gói; thiếu bỏ qua. Số lượng chưa rõ hiện `?`.
- Chọn riêng 34 hạt trồng. Auto mua chỉ theo danh sách mua đã chọn; mua thêm cập nhật kho rồi tiếp tục trồng.
- Bỏ các nút TEST và công cụ thử thủ công khỏi menu; giữ cập nhật kho hạt, đặt lại tiến độ vườn và xem thông tin trồng.
- Giữ thu hoạch Tên Tím/thu thường, lọc trái bị ẩn không cuộn, sửa OK sau bán và mua hạt/công cụ tùy chọn.
- Lần đầu không tự tick. Lưu/khôi phục lựa chọn theo tab LDPlayer; mở lại không tự START.

Chạy EXE một file đã kèm thư viện và ảnh, hoặc giải nén ZIP đầy đủ. Đặt vùng game 960 × 540, quét LDPlayer và chọn đúng tab. Lúc khảo sát đầu tiên hãy để cửa sổ bot/LDPlayer được chọn để chỉnh camera; các lượt dùng sơ đồ đã định vị. STOP nhả cần di chuyển.

## Tự đóng thông báo

Khi START BOT đang chạy, bot kiểm tra hộp thoại **Thông báo** trước khi xử lý ảnh ở các bước thu hoạch, mua/bán và trồng. Chỉ nhận thông báo cây chưa đủ lớn để thu hoạch hoặc nhiệm vụ mới/được cập nhật, với nút OK đơn đã nhận dạng. Không bấm OK chỉ vì có một nút màu xanh; kiểm tra tiêu đề/nội dung và loại trừ xác nhận mua/bán.

Bot đợi hộp thoại ổn định, bấm nút thực tế, kiểm tra đã đóng và chờ giao diện farm/balo/cửa hàng sẵn sàng qua hai ảnh. Thử lại tối đa 3 lần, cách nhau ít nhất 3 giây; sau đó tiếp tục chờ game tải trong giới hạn 2 phút, không spam click. STOP dừng việc chờ. Nếu hộp thoại không đóng hoặc game vẫn chưa sẵn sàng thì bot dừng và báo.

Với **cây chưa đủ lớn**, bot thoát danh sách cũ và chờ 30 giây trước lượt thu hoạch tiếp, giữ lựa chọn và cách thu hoạch của bạn. Với **nhiệm vụ mới**, bot đóng OK rồi tiếp tục luồng đang chạy; không phụ thuộc giờ máy hay tự khởi động lại game.

Kiểm tra v0.1.10: 18 kiểm tra thông báo và 40 kiểm tra cấu hình đạt. Đã thử trên LDPlayer với đúng hộp thoại đang kẹt: bot tự đóng OK, thoát danh sách cũ, báo chờ 30 giây và STOP hoạt động. Lựa chọn ban đầu đã được khôi phục nguyên vẹn. Nhận dạng nhiệm vụ mới được kiểm tra qua OCR với ảnh mô phỏng ghi rõ trong tên tệp; chưa thử thông báo thật lúc 7 giờ. Ba ảnh bán/xác nhận/kết quả thực tế của bản trước không bị nhận nhầm thành thông báo này.

## Kiểm tra luồng ở v0.1.8

145 kiểm tra sơ đồ/di chuyển/bộ nhớ hạt, 55 kiểm tra balo thật, 136 kiểm tra nhận diện trồng và 40 kiểm tra cấu hình đã đạt.

Đã thử trực tiếp trên LDPlayer: khảo sát và đi đến đủ 8/8 luống của vườn hiện tại, không mua hay trồng trong lượt kiểm tra đường đi. Quét balo một lượt, nhớ 12 loại hạt; trồng thử 1 cây Bắp rồi đổi sang Cà rốt và trồng thêm 1 cây. Bộ đếm quét toàn bộ vẫn là 1. Bắp cập nhật 4 → 3, Cà rốt 15 → 14; hai loại được chọn nhưng thiếu (Nho, Dưa) được bỏ qua. Tiến độ giữ 2 cây đã xác nhận. Lựa chọn ban đầu được khôi phục và đối chiếu nguyên vẹn.

Không thử lại toàn bộ giao dịch mua/bán và thu hoạch trong lần cập nhật này. Nhận diện ranh giới cần luống nhìn thấy và đường đi tiếp cận được; kết quả trên vườn hiện tại không chứng minh mọi bố cục khác đều hoạt động.

Xem [hướng dẫn](HUONG-DAN.md), [phân tích luồng](PHAN-TICH-TRONG-BALO.md) và [build/kiểm tra](source/BUILD.md). Mã nguồn ở `source`, ảnh ở `images`, giấy phép thư viện ở `licenses`. Chưa cấp giấy phép riêng cho mã bot.
