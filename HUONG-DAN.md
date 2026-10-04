# BOT FARMER X DLee — v0.1.7

Tên cửa sổ/menu: **BOT FARMER X DLee v0.1**.

## Chạy

Chạy `BOT-FARMER-X-DLee-v0.1.7.exe` (đã kèm DLL/ảnh, không cần quyền quản trị), hoặc giải nén ZIP đầy đủ rồi chạy `autofarmnongtrai-update.exe`.
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

Bot đợi ảnh hạt và vị trí thẻ ổn định sau khi mở/cuộn balo. Ảnh hạt nhỏ được đối chiếu thêm ở các mức phóng gần nhất, giữ nguyên ngưỡng xác nhận để tránh chọn nhầm loại. Gói chưa nhận ra được kiểm tra lại trên cùng trang trước khi kết thúc lượt quét.

## Sửa trồng trong hiệu ứng thời tiết

Hiệu ứng thời tiết làm vòng xanh hợp lệ nhạt màu (độ bão hòa dưới ngưỡng cũ). Bot nhận thêm vòng xanh nhạt, vẫn kiểm tra màu xanh, hình vành tròn, tâm đất nâu, vị trí gần nhân vật và xác nhận kết quả trước khi đếm cây. Không trồng chỉ vì thấy đất nâu.

Ưu tiên khoảng đất nâu đủ rộng trong vùng nhìn thấy để tránh các khe nhỏ giữa cây đã mọc, điều chỉnh theo diện tích đất hiện có. Vẫn cần vòng xanh của game trước khi bấm. Nhận riêng chữ **x1** khi OCR bỏ sót hạt cuối cùng. Chỉ thử lại click một lần nếu hạt/số lượng chưa đổi và vòng xanh vẫn ổn định; nếu chưa xác nhận thì dừng lượt.

START cho lượt trồng hạt có sẵn chạy trước thu hoạch/mua hàng; túi đầy vẫn ưu tiên bán. Mua hạt xong tiếp tục trồng. Hiện **Luot trong gan nhat** riêng để kết quả không bị STATUS mua/bán che mất. START đặt lại thời gian chờ trồng; một lượt chưa về được vườn cũng có thời gian chờ, tránh lặp liên tục.

136 kiểm tra nhận diện/bộ nhớ hạt và 40 kiểm tra cấu hình đã đạt, gồm vòng xanh nhạt từ ảnh game, loại gần xám bị từ chối, điểm đất trên luống Bắp dày, x1/x2 và các số không được nhận nhầm thành 1. Đã thử trên LDPlayer: nhận vòng xanh nhạt, trồng Bắp, START trồng hạt Bắp cuối cùng và xác nhận 1 cây; hai loại hạt đang không có được bỏ qua, kết quả trồng giữ lại khi chuyển sang thu hoạch/bán.

## Sửa OK sau bán

Bot nhận riêng hộp thoại **Hoàn tất bán hàng** và ảnh OK mới. Chỉ hoàn tất lượt bán khi hộp thoại đóng và bảng bán sáng trở lại; nếu click bị mất trong lúc chuyển cảnh thì thử lại có giới hạn. Khi mở chức năng tiếp theo, bot cũng xử lý hộp thoại bán đang còn kẹt.

**TEST DONG OK SAU BAN** chỉ đóng hộp thoại hoàn tất bán. **TEST BAN TU DONG** chạy luồng bán thật theo cách chọn tự động của game. Hai nút chỉ xuất hiện khi bot đang dừng.

15 kiểm tra ảnh bán/OK đạt, gồm tái hiện mẫu OK cũ bị bỏ sót, phân biệt xác nhận bán, đối chiếu bảng bán sau khi đóng OK và tránh coi bảng bán bị làm tối phía sau hộp thoại là đã xong. Đã thử trên LDPlayer: đóng hộp thoại kết quả đang kẹt và chạy toàn bộ luồng bán tự động, tự xác nhận bán/OK rồi thoát về màn hình game.

## Sửa lọc trái thu hoạch

Chế độ **Chon trai** đọc riêng tên trong từng ô nông sản, nhận tên xuống dòng, bỏ dấu tiếng Việt và xử lý lỗi OCR nhỏ khi chỉ có một loại phù hợp. Không dùng khớp một phần để biến Dưa hấu thành Dưa hoặc Táo đường thành Táo. Vị trí bấm/dấu tick lấy từ ô thực tế, rồi kiểm tra đã tick trước khi áp dụng.

**TEST LOC TRAI DA CHON** chỉ thiết lập các dấu tick và để bộ lọc mở cho bạn xem, không bấm thu hoạch hoặc bán. **TEST THU HOACH DA CHON** chạy thu hoạch thật. Dòng trạng thái ghi số loại chọn thành công/tổng loại yêu cầu và tên chưa chọn được. Các trái bị game ẩn vẫn bỏ qua, không cuộn tìm; giữ cách thu Tên Tím hoặc thu thường theo lựa chọn của bạn.

153 kiểm tra tên/lỗi OCR/vị trí ô đã đạt, gồm đọc Bắp, Dâu tây, Nho từ ảnh lọc thực tế và Trăng khuyết/Nhân sâm từ ảnh trước. Đã đối chiếu dấu tick nhiều loại trên LDPlayer và thu riêng Nho bằng Tên Tím thành công; Bắp 14, Dâu tây 3 và Rau xà lách 3 giữ nguyên sau lượt thử. Không thử thu hoạch thật toàn bộ 31 loại hoặc thử lại giao dịch mua/bán trong bản này.

## Lưu lựa chọn và thu hoạch

Lần đầu mọi lựa chọn đều trống. Mỗi thay đổi tự lưu theo tên tab LDPlayer; nút **LUU LUA CHON** lưu thủ công. Lần sau mở/quét lại khôi phục nhưng không tự START. Dữ liệu ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings`, giữ được qua các bản EXE. Đọc được cấu hình v0.1.1; chức năng trồng mới mặc định tắt.
Giữ thu hoạch Tên Tím, thu hoạch thường qua nút Thu hoạch, bỏ cuộn tìm trái bị ẩn, mua hạt/công cụ và bán tùy chọn.

## Kiểm tra

40 kiểm tra lưu cấu hình/chuyển dữ liệu và 130 kiểm tra nhận diện/bộ nhớ balo đã đạt. Kiểm tra đủ 12 gói hạt trong ảnh lọc, bộ nhớ dùng chung cho nhiều hạt có/thiếu, lượt tiếp theo, mua thêm, dùng hết, đổi tab và lượt quét chưa hoàn tất. Luồng trồng đã được thử trên LDPlayer ở v0.1.2; lần cập nhật này kiểm tra việc quét/lấy nhiều loại hạt trực tiếp, không thử lại toàn bộ mua/bán.
Mã nguồn/build ở `source`, tài nguyên ở `images`, giấy phép thư viện ở `licenses`. `SHA256SUMS.txt` đối chiếu ZIP; `.exe.sha256` đối chiếu EXE một file.
