# BOT FARMER X DLee — v0.1.10

Bản v0.1.9 đã bỏ toàn bộ nút TEST, trồng thử một cây, nhóm kiểm tra di chuyển/camera và kiểm tra hàm lẻ trong menu. Các chức năng tự động và lưu lựa chọn giữ nguyên.

Tên cửa sổ/menu: **BOT FARMER X DLee v0.1**.

## Chạy

Chạy `BOT-FARMER-X-DLee-v0.1.10.exe` (đã kèm DLL/ảnh, không cần quyền quản trị), hoặc giải nén ZIP đầy đủ rồi chạy `autofarmnongtrai-update.exe`.
Đặt vùng game LDPlayer 960 × 540, bấm **QUET TAT CA LDPLAYER**, chọn đúng tab, chọn chức năng rồi **START BOT**. **STOP BOT** dừng và nhả cần di chuyển.

## Tự đóng thông báo

Khi START BOT đang chạy, bot kiểm tra hộp thoại **Thông báo** trước khi xử lý ảnh ở các bước thu hoạch, mua/bán và trồng. Chỉ nhận thông báo cây chưa đủ lớn để thu hoạch hoặc nhiệm vụ mới/được cập nhật, với nút OK đơn đã nhận dạng. Không bấm OK chỉ vì có một nút màu xanh; kiểm tra tiêu đề/nội dung và loại trừ xác nhận mua/bán.

Bot đợi hộp thoại ổn định, bấm nút thực tế, kiểm tra đã đóng và chờ giao diện farm/balo/cửa hàng sẵn sàng qua hai ảnh. Thử lại tối đa 3 lần, cách nhau ít nhất 3 giây; sau đó tiếp tục chờ game tải trong giới hạn 2 phút, không spam click. STOP dừng việc chờ. Nếu hộp thoại không đóng hoặc game vẫn chưa sẵn sàng thì bot dừng và báo.

Với **cây chưa đủ lớn**, bot thoát danh sách cũ và chờ 30 giây trước lượt thu hoạch tiếp, giữ lựa chọn và cách thu hoạch của bạn. Với **nhiệm vụ mới**, bot đóng OK rồi tiếp tục luồng đang chạy; không phụ thuộc giờ máy hay tự khởi động lại game.

Kiểm tra v0.1.10: 18 kiểm tra thông báo và 40 kiểm tra cấu hình đạt. Đã thử trên LDPlayer với đúng hộp thoại đang kẹt: bot tự đóng OK, thoát danh sách cũ, báo chờ 30 giây và STOP hoạt động. Lựa chọn ban đầu đã được khôi phục nguyên vẹn. Nhận dạng nhiệm vụ mới được kiểm tra qua OCR với ảnh mô phỏng ghi rõ trong tên tệp; chưa thử thông báo thật lúc 7 giờ. Ba ảnh bán/xác nhận/kết quả thực tế của bản trước không bị nhận nhầm thành thông báo này.

## Auto trồng cây

1. Bật **Auto TRONG CAY (hat trong balo)**.
2. Tick hạt trong **DANH SACH HAT TRONG**. Có 29 hạt thường và 5 hạt đang có trong balo: Hoa chùm sao, Tiểu hành tinh, Cây sao mai, Tulip cam, Rau chân vịt.
3. Bot về nông trại của mình, lấy đúng hạt trong balo, đi từ cổng lên luống đất và tìm vị trí có vòng xanh. Đất nâu hoặc vòng đỏ không đủ điều kiện để bấm trồng.
4. Không có hạt: bỏ qua. Muốn mua rồi trồng: bật **Auto MUA HAT**, chọn hạt trong danh sách mua và đồng thời chọn trong danh sách trồng. Bot chỉ mua các hạt bạn đã chọn; mua xong quay lại vườn trồng.

Mỗi lượt có giới hạn 240 cây và 10 phút, nghỉ ít nhất 60 giây giữa các lượt. Bot dùng sơ đồ các luống đã khảo sát và giữ tiến độ nếu cần tiếp tục.
Chọn chức năng/hạt rồi bấm **START BOT**; bot tự quét balo và khảo sát vườn khi cần.

## Nhớ hạt trong balo

Bot quét toàn bộ danh sách hạt một lượt rồi nhớ cả loại có và loại thiếu cho tab LDPlayer đang chạy. Chọn nhiều hạt dùng chung dữ liệu này; các lượt trồng tiếp theo không tìm lại từng loại. Khi đổi hạt, bot vẫn mở balo để lấy hạt tại vị trí đã nhớ và xác nhận đúng loại trước khi bấm.

Mua thêm hạt thành công sẽ làm mới dữ liệu cho lượt trồng tiếp theo. Hạt dùng hết được đánh dấu thiếu, vẫn giữ dữ liệu các loại khác. Nếu vị trí gói hạt thay đổi, bot kiểm tra lại trang đang mở; chỉ quét toàn bộ lại khi dữ liệu vị trí không còn đúng.

Nút **QUET LAI HAT TRONG BALO** cập nhật khi bạn tự thay đổi đồ. Dòng **So luot quet** cho biết số lượt quét toàn bộ, không phải số hạt đã chọn. Dữ liệu balo chỉ nhớ trong lần chạy hiện tại; mỗi lần START hoặc mở lại EXE sẽ kiểm tra tồn kho mới. Danh sách đã chọn vẫn tự lưu như trước.

Bot đợi ảnh hạt và vị trí thẻ ổn định sau khi mở/cuộn balo. Ảnh hạt nhỏ được đối chiếu thêm ở các mức phóng gần nhất, giữ nguyên ngưỡng xác nhận để tránh chọn nhầm loại. Gói chưa nhận ra được kiểm tra lại trên cùng trang trước khi kết thúc lượt quét.

## Trồng theo sơ đồ vườn và quản lý hạt

Bot đọc tên, ảnh, số lượng và vị trí gói hạt trong tab Hạt giống của My Farm. Quét hết danh sách một lượt; không lấy công cụ hoặc trái làm hạt. Số lượng chưa đọc được hiện `?`, không đoán thành 0. Khi đổi hạt, kiểm tra trang đang mở và bấm vị trí đã nhớ, xác nhận đúng hạt trong tay. Mua thêm/START/quét thủ công cập nhật balo; gói dùng hết được cập nhật riêng, giữ dữ liệu các gói còn lại.

Hạt đang cầm và được chọn trồng trước; các hạt có sẵn còn lại theo thứ tự vị trí trong balo để giảm đi lại giữa các trang. Hạt thiếu/chưa đọc rõ được báo và bỏ qua. Chỉ mua các hạt trong danh sách mua của bạn khi Auto MUA HAT bật.

Trước lượt trồng, bot về cổng vườn để xác định điểm gốc, khảo sát ranh giới các luống dọc lối giữa và lập đường đi qua từng luống. Bot giữ nguyên góc camera đã khảo sát, xác minh lại vị trí ở cổng vườn rồi theo dõi dịch chuyển mặt đất để giữ tọa độ sơ đồ. Có chuẩn hóa độ sáng khi trời chuyển tối/mưa. Đổi loại hạt tiếp tục trên sơ đồ hiện tại; không về nhà cho từng hạt. Chỉ trồng khi thấy vòng xanh hợp lệ và xác nhận hạt đã tiêu thụ. Điểm không phù hợp với một loại vẫn có thể thử loại khác; điểm đã trồng được bỏ qua.

**DAT LAI TIEN DO VUON** bỏ tiến độ khi bạn đổi bố cục; bot tự khảo sát lại trong lượt trồng tiếp theo. Dòng Vườn hiển thị số luống, điểm đã kiểm tra và cây đã xác nhận. **HAT DA NHO TRONG BALO** xem các loại/số lượng đã đọc.

Mỗi lượt có giới hạn 10 phút và 240 cây, giữ tiến độ khi cần tiếp tục. Nếu mất dấu camera, bot thử khảo sát lại một lần; vẫn không định vị được thì dừng lượt và báo. Không tự lưu/chỉnh sửa đồ đạc khi game đang trong chế độ chỉnh sửa vườn. Cần ranh giới luống nhìn thấy và lối đi có thể tiếp cận; không đảm bảo nhận đúng mọi bố cục khác.

## Sửa trồng trong hiệu ứng thời tiết

Hiệu ứng thời tiết làm vòng xanh hợp lệ nhạt màu (độ bão hòa dưới ngưỡng cũ). Bot nhận thêm vòng xanh nhạt, vẫn kiểm tra màu xanh, hình vành tròn, tâm đất nâu, vị trí gần nhân vật và xác nhận kết quả trước khi đếm cây. Không trồng chỉ vì thấy đất nâu.

Ưu tiên khoảng đất nâu đủ rộng trong vùng nhìn thấy để tránh các khe nhỏ giữa cây đã mọc, điều chỉnh theo diện tích đất hiện có. Vẫn cần vòng xanh của game trước khi bấm. Nhận riêng chữ **x1** khi OCR bỏ sót hạt cuối cùng. Chỉ thử lại click một lần nếu hạt/số lượng chưa đổi và vòng xanh vẫn ổn định; nếu chưa xác nhận thì dừng lượt.

START cho lượt trồng hạt có sẵn chạy trước thu hoạch/mua hàng; túi đầy vẫn ưu tiên bán. Mua hạt xong tiếp tục trồng. Hiện **Luot trong gan nhat** riêng để kết quả không bị STATUS mua/bán che mất. START đặt lại thời gian chờ trồng; một lượt chưa về được vườn cũng có thời gian chờ, tránh lặp liên tục.

136 kiểm tra nhận diện/bộ nhớ hạt và 40 kiểm tra cấu hình đã đạt, gồm vòng xanh nhạt từ ảnh game, loại gần xám bị từ chối, điểm đất trên luống Bắp dày, x1/x2 và các số không được nhận nhầm thành 1. Đã thử trên LDPlayer: nhận vòng xanh nhạt, trồng Bắp, START trồng hạt Bắp cuối cùng và xác nhận 1 cây; hai loại hạt đang không có được bỏ qua, kết quả trồng giữ lại khi chuyển sang thu hoạch/bán.

## Sửa OK sau bán

Bot nhận riêng hộp thoại **Hoàn tất bán hàng** và ảnh OK mới. Chỉ hoàn tất lượt bán khi hộp thoại đóng và bảng bán sáng trở lại; nếu click bị mất trong lúc chuyển cảnh thì thử lại có giới hạn. Khi mở chức năng tiếp theo, bot cũng xử lý hộp thoại bán đang còn kẹt.


15 kiểm tra ảnh bán/OK đạt, gồm tái hiện mẫu OK cũ bị bỏ sót, phân biệt xác nhận bán, đối chiếu bảng bán sau khi đóng OK và tránh coi bảng bán bị làm tối phía sau hộp thoại là đã xong. Đã thử trên LDPlayer: đóng hộp thoại kết quả đang kẹt và chạy toàn bộ luồng bán tự động, tự xác nhận bán/OK rồi thoát về màn hình game.

## Sửa lọc trái thu hoạch

Chế độ **Chon trai** đọc riêng tên trong từng ô nông sản, nhận tên xuống dòng, bỏ dấu tiếng Việt và xử lý lỗi OCR nhỏ khi chỉ có một loại phù hợp. Không dùng khớp một phần để biến Dưa hấu thành Dưa hoặc Táo đường thành Táo. Vị trí bấm/dấu tick lấy từ ô thực tế, rồi kiểm tra đã tick trước khi áp dụng.


153 kiểm tra tên/lỗi OCR/vị trí ô đã đạt, gồm đọc Bắp, Dâu tây, Nho từ ảnh lọc thực tế và Trăng khuyết/Nhân sâm từ ảnh trước. Đã đối chiếu dấu tick nhiều loại trên LDPlayer và thu riêng Nho bằng Tên Tím thành công; Bắp 14, Dâu tây 3 và Rau xà lách 3 giữ nguyên sau lượt thử. Không thử thu hoạch thật toàn bộ 31 loại hoặc thử lại giao dịch mua/bán trong bản này.

## Lưu lựa chọn và thu hoạch

Lần đầu mọi lựa chọn đều trống. Mỗi thay đổi tự lưu theo tên tab LDPlayer; nút **LUU LUA CHON** lưu thủ công. Lần sau mở/quét lại khôi phục nhưng không tự START. Dữ liệu ở `%LOCALAPPDATA%/BOT FARMER X DLee/settings`, giữ được qua các bản EXE. Đọc được cấu hình v0.1.1; chức năng trồng mới mặc định tắt.
Giữ thu hoạch Tên Tím, thu hoạch thường qua nút Thu hoạch, bỏ cuộn tìm trái bị ẩn, mua hạt/công cụ và bán tùy chọn.

## Kiểm tra luồng ở v0.1.8

145 kiểm tra sơ đồ/di chuyển/bộ nhớ hạt, 55 kiểm tra balo thật, 136 kiểm tra nhận diện trồng và 40 kiểm tra cấu hình đã đạt.

Đã thử trực tiếp trên LDPlayer: khảo sát và đi đến đủ 8/8 luống của vườn hiện tại, không mua hay trồng trong lượt kiểm tra đường đi. Quét balo một lượt, nhớ 12 loại hạt; trồng thử 1 cây Bắp rồi đổi sang Cà rốt và trồng thêm 1 cây. Bộ đếm quét toàn bộ vẫn là 1. Bắp cập nhật 4 → 3, Cà rốt 15 → 14; hai loại được chọn nhưng thiếu (Nho, Dưa) được bỏ qua. Tiến độ giữ 2 cây đã xác nhận. Lựa chọn ban đầu được khôi phục và đối chiếu nguyên vẹn.

Không thử lại toàn bộ giao dịch mua/bán và thu hoạch trong lần cập nhật này. Nhận diện ranh giới cần luống nhìn thấy và đường đi tiếp cận được; kết quả trên vườn hiện tại không chứng minh mọi bố cục khác đều hoạt động.

Mã nguồn/build ở `source`, tài nguyên ở `images`, giấy phép thư viện ở `licenses`. `SHA256SUMS.txt` đối chiếu ZIP; `.exe.sha256` đối chiếu EXE một file.
