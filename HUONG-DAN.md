# BOT FARMER X DLee — v0.1.3

Tên cửa sổ/menu: **BOT FARMER X DLee v0.1**.

## Chạy

Chạy `BOT-FARMER-X-DLee-v0.1.3.exe` (đã kèm DLL/ảnh, không cần quyền quản trị), hoặc giải nén ZIP đầy đủ rồi chạy `autofarmnongtrai-update.exe`.
Đặt vùng game LDPlayer 960 × 540, bấm **QUET TAT CA LDPLAYER**, chọn đúng tab, chọn chức năng rồi **START BOT**. **STOP BOT** dừng và nhả cần di chuyển.

## Auto trồng cây

1. Bật **Auto TRONG CAY (hat trong balo)**.
2. Tick hạt trong **DANH SACH HAT TRONG**. Có 29 hạt thường và 5 hạt đang có trong balo: Hoa chùm sao, Tiểu hành tinh, Cây sao mai, Tulip cam, Rau chân vịt.
3. Bot về nông trại của mình, lấy đúng hạt trong balo, đi từ cổng lên luống đất và tìm vị trí có vòng xanh. Đất nâu hoặc vòng đỏ không đủ điều kiện để bấm trồng.
4. Không có hạt: bỏ qua. Muốn mua rồi trồng: bật **Auto MUA HAT**, chọn hạt trong danh sách mua và đồng thời chọn trong danh sách trồng. Bot chỉ mua các hạt bạn đã chọn; mua xong quay lại vườn trồng.

Mỗi lượt giới hạn 40 cây và thời gian tìm đường, nghỉ ít nhất 60 giây trước lượt tiếp. Bot chỉ tìm đất trong vùng đang nhìn thấy, không đảm bảo phủ toàn bộ vườn có bố cục khác. Nếu không xác nhận được kết quả trồng hoặc bị kẹt, dừng lượt và báo trạng thái.
**TRONG THU 1 CAY** chạy một cây trước khi bật liên tục. Nhóm kiểm tra di chuyển có các nút đi 1 giây, lấy hạt, camera gần hơn và đọc vòng xanh/số hạt.

## Nhớ hạt trong balo

Bot quét toàn bộ danh sách hạt một lượt rồi nhớ cả loại có và loại thiếu cho tab LDPlayer đang chạy. Chọn nhiều hạt dùng chung dữ liệu này; các lượt trồng tiếp theo không tìm lại từng loại. Khi đổi hạt, bot vẫn mở balo để lấy hạt tại vị trí đã nhớ và xác nhận đúng loại trước khi bấm.

Mua thêm hạt thành công sẽ làm mới dữ liệu cho lượt trồng tiếp theo. Hạt dùng hết được đánh dấu thiếu, vẫn giữ dữ liệu các loại khác. Nếu vị trí gói hạt thay đổi, bot kiểm tra lại trang đang mở; chỉ quét toàn bộ lại khi dữ liệu vị trí không còn đúng.

Nút **QUET LAI HAT TRONG BALO** cập nhật khi bạn tự thay đổi đồ. Dòng **So luot quet** cho biết số lượt quét toàn bộ, không phải số hạt đã chọn. Dữ liệu balo chỉ nhớ trong lần chạy hiện tại; mỗi lần START hoặc mở lại EXE sẽ kiểm tra tồn kho mới. Danh sách đã chọn vẫn tự lưu như trước.

## Lưu lựa chọn và thu hoạch

Lần đầu mọi lựa chọn đều trống. Mỗi thay đổi tự lưu theo tên tab LDPlayer; nút **LUU LUA CHON** lưu thủ công. Lần sau mở/quét lại khôi phục nhưng không tự START. Dữ liệu ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings`, giữ được qua các bản EXE. Đọc được cấu hình v0.1.1; chức năng trồng mới mặc định tắt.
Giữ thu hoạch Tên Tím, thu hoạch thường qua nút Thu hoạch, bỏ cuộn tìm trái bị ẩn, mua hạt/công cụ và bán tùy chọn.

## Kiểm tra

40 kiểm tra lưu cấu hình/chuyển dữ liệu và 102 kiểm tra nhận diện/bộ nhớ balo đã đạt. Kiểm tra bộ nhớ bằng nhiều hạt có/thiếu, lượt tiếp theo, mua thêm, dùng hết, đổi tab và lượt quét chưa hoàn tất. Luồng trồng đã được thử trên LDPlayer ở v0.1.2; lần cập nhật này kiểm tra việc quét/lấy nhiều loại hạt trực tiếp, không thử lại toàn bộ mua/bán.
Mã nguồn/build ở `source`, tài nguyên ở `images`, giấy phép thư viện ở `licenses`. `SHA256SUMS.txt` đối chiếu ZIP; `.exe.sha256` đối chiếu EXE một file.
