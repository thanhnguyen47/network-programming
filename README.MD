# Sử dụng netcat để gửi, nhận dữ liệu đơn giản.
## Dùng netcat như một **TCP server**: 
* nc -lvp <*cổng đợi kết nối*>
  * nc -lvp 9000
## Dùng netcat như một **TCP client**:
* nc -v <*ip/tên miền*> <*cổng*> 
  * nc -v localhost 9000
## Dùng netcat như một **UDP receiver**:
* nc -v -l -u -p <*cổng đợi kết nối*>
  * nc -lvup 9000
## Dùng netcat như một **UDP sender**:
* nc -v -u <*ip/tên miền> <*cổng*>
  * nc -v -u localhost 9000