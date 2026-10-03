# BOT FARMER X DLee — v0.1.2

Tên cửa sổ/menu: **BOT FARMER X DLee v0.1**.

## Chạy

Chạy `BOT-FARMER-X-DLee-v0.1.2.exe` (đã kèm DLL/ảnh, không cần quyền quản trị), hoặc giải nén ZIP đầy đủ rồi chạy `autofarmnongtrai-update.exe`.
Đặt vùng game LDPlayer 960 × 540, bấm **QUET TAT CA LDPLAYER**, chọn đúng tab, chọn chức năng rồi **START BOT**. **STOP BOT** dừng và nhả cần di chuyển.

## Auto trồng cây

1. Bật **Auto TRONG CAY (hat trong balo)**.
2. Tick hạt trong **DANH SACH HAT TRONG**. Có 29 hạt thường và 5 hạt đang có trong balo: Hoa chùm sao, Tiểu hành tinh, Cây sao mai, Tulip cam, Rau chân vịt.
3. Bot về nông trại của mình, lấy đúng hạt trong balo, đi từ cổng lên luống đất và tìm vị trí có vòng xanh. Đất nâu hoặc vòng đỏ không đủ điều kiện để bấm trồng.
4. Không có hạt: bỏ qua. Muốn mua rồi trồng: bật **Auto MUA HAT**, chọn hạt trong danh sách mua và đồng thời chọn trong danh sách trồng. Bot chỉ mua các hạt bạn đã chọn; mua xong quay lại vườn trồng.

Mỗi lượt giới hạn 40 cây và thời gian tìm đường, nghỉ ít nhất 60 giây trước lượt tiếp. Bot chỉ tìm đất trong vùng đang nhìn thấy, không đảm bảo phủ toàn bộ vườn có bố cục khác. Nếu không xác nhận được kết quả trồng hoặc bị kẹt, dừng lượt và báo trạng thái.
**TRONG THU 1 CAY** chạy một cây trước khi bật liên tục. Nhóm kiểm tra di chuyển có các nút đi 1 giây, lấy hạt, camera gần hơn và đọc vòng xanh/số hạt.

## Lưu lựa chọn và thu hoạch

Lần đầu mọi lựa chọn đều trống. Mỗi thay đổi tự lưu theo tên tab LDPlayer; nút **LUU LUA CHON** lưu thủ công. Lần sau mở/quét lại khôi phục nhưng không tự START. Dữ liệu ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings`, giữ được qua các bản EXE. Đọc được cấu hình v0.1.1; chức năng trồng mới mặc định tắt.
Giữ thu hoạch Tên Tím, thu hoạch thường qua nút Thu hoạch, bỏ cuộn tìm trái bị ẩn, mua hạt/công cụ và bán tùy chọn.

## Kiểm tra

39 kiểm tra lưu cấu hình và chuyển dữ liệu v0.1.1; 59 kiểm tra nhận diện hạt, vòng xanh/vòng đỏ, đất đêm và theo dõi camera. Đã thử lấy cà rốt, di chuyển và trồng trên tab `(clone)`. Chi tiết xác nhận lượt trồng được ghi trong bản phát hành. Không thử lại toàn bộ mua/bán trong lần cập nhật này.
Mã nguồn/build ở `source`, tài nguyên ở `images`, giấy phép thư viện ở `licenses`. `SHA256SUMS.txt` đối chiếu ZIP; `.exe.sha256` đối chiếu EXE một file.
