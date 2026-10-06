# Phân tích trồng và balo — v0.1.8

Vườn thực tế có hai dãy, mỗi dãy bốn luống. Tìm đất chỉ trong một khung hình bỏ sót phần trên của vườn. Quét balo theo từng hạt và đặt lại trang khiến chọn nhiều loại lặp lại thao tác.

Luồng mới:

1. Xác nhận My Farm → Hạt giống đã được chọn bằng nền nút; chữ sáng ở tab chưa chọn không được xem là đã chọn. Đợi thẻ ổn định rồi đọc tên, đối chiếu ảnh hạt và đọc số lượng. Nhớ trang, vị trí, loại có/thiếu trong một lượt quét. Số chưa đọc rõ hiện `?`.
2. Xác định điểm gốc ở cổng vườn, khảo sát dọc lối giữa và ghép ranh giới luống. Loại đường giả do điện thoại/balo che lên luống. Giữ camera đã khảo sát, xác minh lại ở cổng vườn. Theo dõi mặt đất với chuẩn hóa sáng và kiểm tra hình học khi trời tối/mưa.
3. Đi từ các luống gần cổng lên trên, mỗi chuyển động ngắn được đo lại trước bước tiếp. Mỗi luống có nhiều hàng điểm thử; chỉ bấm khi vòng xanh ổn định nằm trong luống đã định vị. Xác nhận tiêu thụ hạt/kết quả trước khi đếm cây.
4. Hạt đang cầm và được chọn được ưu tiên, rồi đến hạt có sẵn theo trang balo. Khi đổi loại, mở đúng trang đã nhớ, kiểm tra ảnh và xác nhận hạt trong tay. Không quét toàn bộ lại chỉ vì đổi hạt. Mua thêm hoặc dữ liệu vị trí không còn đúng mới cần cập nhật kho.
5. Điểm đã trồng được bỏ qua cho mọi hạt. Điểm không phù hợp một hạt có thể thử loại khác. Giữ tiến độ qua lượt bị giới hạn; mất định vị hoặc kết quả chưa rõ thì dừng, không bấm lặp vô hạn.

## Kiểm tra v0.1.8

145 kiểm tra sơ đồ/di chuyển/bộ nhớ hạt, 55 kiểm tra balo thật, 136 kiểm tra nhận diện trồng và 40 kiểm tra cấu hình đã đạt.

Đã thử trực tiếp trên LDPlayer: khảo sát và đi đến đủ 8/8 luống của vườn hiện tại, không mua hay trồng trong lượt kiểm tra đường đi. Quét balo một lượt, nhớ 12 loại hạt; trồng thử 1 cây Bắp rồi đổi sang Cà rốt và trồng thêm 1 cây. Bộ đếm quét toàn bộ vẫn là 1. Bắp cập nhật 4 → 3, Cà rốt 15 → 14; hai loại được chọn nhưng thiếu (Nho, Dưa) được bỏ qua. Tiến độ giữ 2 cây đã xác nhận. Lựa chọn ban đầu được khôi phục và đối chiếu nguyên vẹn.

Không thử lại toàn bộ giao dịch mua/bán và thu hoạch trong lần cập nhật này. Nhận diện ranh giới cần luống nhìn thấy và đường đi tiếp cận được; kết quả trên vườn hiện tại không chứng minh mọi bố cục khác đều hoạt động.
