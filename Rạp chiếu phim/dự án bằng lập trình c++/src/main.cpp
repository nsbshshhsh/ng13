// ============================================================
//  HỆ THỐNG ĐẶT VÉ XEM PHIM - CINEMA BOOKING SYSTEM
//  Phiên bản: 1.0 | Năm: 2024
//  Tác giả  : [Tên của bạn]
//
//  Kiến trúc: C++ OOP với đầy đủ các lớp:
//    User → Customer, Admin
//    Movie, CinemaRoom, Room, Seat
//    Showtime, Ticket, Booking
//    MovieSchedule (quản lý lịch chiếu)
//    PaymentService (thanh toán + hoàn tiền)
// ============================================================

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// Models
#include "models/Admin.h"
#include "models/Booking.h"
#include "models/CinemaRoom.h"
#include "models/Customer.h"
#include "models/Movie.h"
#include "models/MovieSchedule.h"
#include "models/Room.h"
#include "models/Seat.h"
#include "models/Showtime.h"
#include "models/Ticket.h"

// Services
#include "services/PaymentService.h"
#include "utils/DatabaseManager.h"

// ============================================================
// Dữ liệu mẫu toàn cục (trong thực tế lưu file/DB)
// ============================================================
static std::vector<Movie> danhSachPhim;
static std::vector<Customer> danhSachKhachHang;
static std::vector<Admin> danhSachAdmin;
static std::vector<Booking> danhSachDon;
static CinemaRoom rap;
static MovieSchedule lichChieu;
static PaymentService paymentSvc;

// Hàm lưu dữ liệu
void luuDuLieu() {
  DatabaseManager::saveAllData(danhSachPhim, rap, danhSachKhachHang,
                               danhSachAdmin, danhSachDon, lichChieu);
}

// Con trỏ người dùng đang đăng nhập
static Customer *khachHienTai = nullptr;
static Admin *adminHienTai = nullptr;

// ============================================================
// Hàm tiện ích: xóa màn hình, nhập số an toàn
// ============================================================
void xoaManHinh() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

// Nhập số nguyên an toàn (tránh crash khi nhập sai)
int nhapSoNguyen(const std::string &thongBao) {
  int gia_tri;
  while (true) {
    std::cout << thongBao;
    if (std::cin >> gia_tri) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return gia_tri;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "⚠️  Vui lòng nhập số nguyên hợp lệ!\n";
  }
}

// Nhập chuỗi an toàn
std::string nhapChuoi(const std::string &thongBao) {
  std::cout << thongBao;
  std::string gia_tri;
  std::getline(std::cin, gia_tri);
  return gia_tri;
}

// ============================================================
// KHỞI TẠO DỮ LIỆU MẪU
// ============================================================
void khoiTaoDuLieuMau() {
  DatabaseManager::seedDefaultData(danhSachPhim, rap, danhSachKhachHang,
                                   danhSachAdmin, lichChieu);

  // Đồng bộ ngược từ lichChieu về các biến toàn cục khác (Bị tắt để tránh lỗi
  // dangling pointer cho phongChieu trong Showtime)
  // rap.getDanhSachPhong().clear();
  // for (auto* p : lichChieu.getDanhSachPhongRef()) {
  //     if (p) rap.getDanhSachPhong().push_back(*p);
  // }

  // Nạp danh sách đơn hàng đã lưu
  danhSachDon = DatabaseManager::loadBookingsAndTickets(
      danhSachKhachHang, lichChieu.getDanhSachSuatChieu());
}

// ============================================================
// MENU ĐĂNG NHẬP
// ============================================================
bool menuDangNhap() {
  std::cout << "\n┌──────────────────────────────────────┐\n";
  std::cout << "│         🎬  ĐĂNG NHẬP HỆ THỐNG        │\n";
  std::cout << "└──────────────────────────────────────┘\n";
  std::string email = nhapChuoi("  📧 Email    : ");
  std::string matKhau = nhapChuoi("  🔒 Mật khẩu : ");

  // Kiểm tra trong danh sách Admin
  for (auto &admin : danhSachAdmin) {
    if (admin.dangNhap(email, matKhau)) {
      adminHienTai = &admin;
      khachHienTai = nullptr;
      return true;
    }
  }

  // Kiểm tra trong danh sách Khách hàng
  for (auto &kh : danhSachKhachHang) {
    if (kh.dangNhap(email, matKhau)) {
      khachHienTai = &kh;
      adminHienTai = nullptr;
      return true;
    }
  }

  std::cout << "❌ Email hoặc mật khẩu không đúng!\n";
  return false;
}

// ============================================================
// ĐĂNG KÝ TÀI KHOẢN MỚI
// ============================================================
void menuDangKy() {
  std::cout << "\n┌──────────────────────────────────────┐\n";
  std::cout << "│         📝  ĐĂNG KÝ TÀI KHOẢN         │\n";
  std::cout << "└──────────────────────────────────────┘\n";

  std::string hoTen = nhapChuoi("  Họ và tên  : ");
  std::string email = nhapChuoi("  Email      : ");

  // Kiểm tra email đã tồn tại chưa
  for (const auto &kh : danhSachKhachHang) {
    if (kh.getEmail() == email) {
      std::cout << "❌ Email này đã được đăng ký!\n";
      return;
    }
  }

  std::string sdt = nhapChuoi("  Số điện thoại: ");
  std::string mk = nhapChuoi("  Mật khẩu   : ");
  std::string mkXn = nhapChuoi("  Nhập lại MK : ");

  if (mk != mkXn) {
    std::cout << "❌ Mật khẩu không khớp!\n";
    return;
  }
  if (mk.length() < 6) {
    std::cout << "❌ Mật khẩu phải có ít nhất 6 ký tự!\n";
    return;
  }

  int idMoi = 200 + (int)danhSachKhachHang.size() + 1;
  danhSachKhachHang.emplace_back(idMoi, hoTen, email, sdt, mk);
  luuDuLieu();
  std::cout << "✅ Đăng ký thành công! Chào mừng, " << hoTen << "!\n";
  std::cout << "   Bạn có thể đăng nhập ngay bây giờ.\n";
}

// ============================================================
// MENU KHÁCH HÀNG
// ============================================================

// 1. Xem danh sách phim đang chiếu
void menuXemPhim() {
  std::cout << "\n🎬 DANH SÁCH PHIM ĐANG CHIẾU\n";
  std::cout
      << "════════════════════════════════════════════════════════════════\n";
  std::cout << std::left << std::setw(8) << "Mã" << std::setw(30) << "Tên phim"
            << std::setw(15) << "Thể loại" << std::setw(12) << "Thời lượng"
            << std::setw(5) << "T.hạn"
            << "Đánh giá\n";
  std::cout << std::string(80, '─') << "\n";

  bool coPhim = false;
  for (const auto &phim : danhSachPhim) {
    if (phim.isDangChieu()) {
      phim.hienThiTomTat();
      coPhim = true;
    }
  }
  if (!coPhim)
    std::cout << "  (Chưa có phim nào đang chiếu)\n";
  std::cout
      << "════════════════════════════════════════════════════════════════\n";
}

// 2. Xem chi tiết phim
void menuXemChiTietPhim() {
  std::string maPhim = nhapChuoi("Nhập mã phim (VD: MV001): ");
  for (const auto &phim : danhSachPhim) {
    if (phim.getMaPhim() == maPhim) {
      phim.hienThiThongTin();
      return;
    }
  }
  std::cout << "❌ Không tìm thấy phim: " << maPhim << "\n";
}

// 3. Xem lịch chiếu theo phim
void menuXemLichChieu() {
  std::string ngay = nhapChuoi("Nhập ngày xem (YYYY-MM-DD, Enter = hôm nay): ");
  if (ngay.empty()) {
    time_t t = time(nullptr);
    tm *tmInfo = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tmInfo);
    ngay = std::string(buf);
  }
  lichChieu.hienThiLichChieuTheoNgay(ngay);
}

// 4. Đặt vé
void menuDatVe() {
  if (!khachHienTai) {
    std::cout << "❌ Vui lòng đăng nhập trước!\n";
    return;
  }

  // Bước 1: Chọn suất chiếu
  std::string maSuat = nhapChuoi("Nhập mã suất chiếu: ");
  Showtime *suat = lichChieu.timSuatChieuBangMa(maSuat);

  if (!suat) {
    std::cout << "❌ Không tìm thấy suất chiếu!\n";
    return;
  }
  if (!suat->coTheDatVe()) {
    std::cout << "❌ Suất chiếu không còn vé hoặc đã đóng bán!\n";
    return;
  }

  // Hiển thị thông tin suất và sơ đồ ghế
  suat->hienThiChiTiet();
  if (suat->getPhongChieu()) {
    suat->getPhongChieu()->hienThiSoDo();
  }

  // Bước 2: Chọn ghế
  std::cout << "Nhập mã ghế (VD: A01 B02 C03, cách nhau bằng dấu cách): ";
  std::string dongGhe;
  std::getline(std::cin, dongGhe);

  // Tách các mã ghế
  std::vector<std::string> danhSachMaGhe;
  std::istringstream iss(dongGhe);
  std::string maGhe;
  while (iss >> maGhe) {
    danhSachMaGhe.push_back(maGhe);
  }

  if (danhSachMaGhe.empty()) {
    std::cout << "❌ Chưa chọn ghế!\n";
    return;
  }

  // Tạo đơn đặt vé
  Booking donMoi(khachHienTai, PhuongThucThanhToan::VNPAY);
  Room *phong = suat->getPhongChieu();

  // Tạo từng vé
  std::vector<Ticket> danhSachVe;
  static int soThuTuVe = 1;
  bool tatCaHopLe = true;

  for (const auto &maGheLua : danhSachMaGhe) {
    Seat *ghe = phong ? phong->timGheBangMa(maGheLua) : nullptr;
    if (!ghe) {
      std::cout << "❌ Ghế " << maGheLua << " không tồn tại!\n";
      tatCaHopLe = false;
      break;
    }
    if (!ghe->coTheDat()) {
      std::cout << "❌ Ghế " << maGheLua << " đã được đặt!\n";
      tatCaHopLe = false;
      break;
    }

    std::string maVe = Ticket::taoMaVeMoi(soThuTuVe++);
    Ticket ve(maVe, suat, ghe, khachHienTai->getHoTen(),
              khachHienTai->getEmail(), donMoi.getMaDon());
    danhSachVe.push_back(ve);
  }

  if (!tatCaHopLe || danhSachVe.empty())
    return;

  // Hiển thị tổng tiền trước khi thanh toán
  donMoi.taoDonDat(danhSachVe);
  donMoi.tinhTongTien();

  std::cout << "\n💰 Tổng tiền: " << std::fixed << std::setprecision(0)
            << donMoi.getTongTien() << "đ\n";

  // Hỏi dùng điểm tích lũy không
  if (khachHienTai->getDiemTichLuy() >= 100) {
    std::cout << "⭐ Bạn có " << khachHienTai->getDiemTichLuy()
              << " điểm. Dùng điểm để giảm giá? (y/n): ";
    char dung;
    std::cin >> dung;
    std::cin.ignore();
    if (dung == 'y' || dung == 'Y') {
      int diemDung = nhapSoNguyen("Nhập số điểm muốn dùng (bội số 100): ");
      double tienGiam = khachHienTai->doiDiem(diemDung);
      if (tienGiam > 0)
        donMoi.apDungGiamGia(tienGiam);
    }
  }

  // Chọn phương thức thanh toán
  std::cout << "\n📱 PHƯƠNG THỨC THANH TOÁN:\n";
  std::cout << "  1. Tiền mặt\n";
  std::cout << "  2. Thẻ ngân hàng\n";
  std::cout << "  3. Ví MoMo\n";
  std::cout << "  4. VNPay QR\n";
  std::cout << "  5. ZaloPay\n";
  int luaPTTT = nhapSoNguyen("Chọn (1-5): ");

  PhuongThucThanhToan pttt = PhuongThucThanhToan::TIEN_MAT;
  switch (luaPTTT) {
  case 2:
    pttt = PhuongThucThanhToan::THE_NGAN_HANG;
    break;
  case 3:
    pttt = PhuongThucThanhToan::MOMO;
    break;
  case 4:
    pttt = PhuongThucThanhToan::VNPAY;
    break;
  case 5:
    pttt = PhuongThucThanhToan::ZALOPAY;
    break;
  default:
    pttt = PhuongThucThanhToan::TIEN_MAT;
    break;
  }

  // Đặt ghế trước
  if (phong) {
    phong->datNhieuGhe(danhSachMaGhe, donMoi.getMaDon());
  }

  // Xử lý thanh toán
  if (paymentSvc.xuLyThanhToan(donMoi, pttt)) {
    // In vé từng cái
    std::cout << "\n📄 VÉ CỦA BẠN:\n";
    for (auto &ve : donMoi.getDanhSachVe()) {
      ve.inVe();
    }
    danhSachDon.push_back(donMoi);
    luuDuLieu();
  } else {
    // Thanh toán thất bại → giải phóng ghế
    if (phong)
      phong->huyNhieuGhe(danhSachMaGhe);
    std::cout << "❌ Thanh toán thất bại. Vui lòng thử lại!\n";
  }
}

// 5. Xem lịch sử vé
void menuXemLichSu() {
  if (!khachHienTai)
    return;
  khachHienTai->xemLichSu();

  if (!danhSachDon.empty()) {
    std::cout << "\nChi tiết đơn hàng:\n";
    for (const auto &don : danhSachDon) {
      if (don.getKhachHang() &&
          don.getKhachHang()->getId() == khachHienTai->getId()) {
        don.hienThiChiTiet();
      }
    }
  }
}

// 6. Hủy vé
void menuHuyVe() {
  if (!khachHienTai)
    return;

  std::string maDon = nhapChuoi("Nhập mã đơn cần hủy (VD: BK-20241115-0001): ");

  for (auto &don : danhSachDon) {
    if (don.getMaDon() == maDon && don.getKhachHang() &&
        don.getKhachHang()->getId() == khachHienTai->getId()) {

      if (!don.coTheHuy()) {
        std::cout << "❌ Đơn này không thể hủy!\n";
        return;
      }

      // Tính giờ trước chiếu (demo: hỏi user)
      double gioTruocChieu = 30.0; // Giả lập 30h trước chiếu
      std::cout << "⏰ Thời gian còn lại trước chiếu: " << gioTruocChieu
                << "h\n";

      double tienHoan = paymentSvc.xuLyHoanTien(don, gioTruocChieu);
      don.huyDon();

      // Giải phóng ghế trong phòng chiếu
      std::vector<std::string> seatNames;
      for (const auto &t : don.getDanhSachVe()) {
        if (t.getGhe())
          seatNames.push_back(t.getGhe()->getMaGhe());
      }
      if (!don.getDanhSachVe().empty() &&
          don.getDanhSachVe()[0].getSuatChieu() &&
          don.getDanhSachVe()[0].getSuatChieu()->getPhongChieu()) {
        don.getDanhSachVe()[0].getSuatChieu()->getPhongChieu()->huyNhieuGhe(
            seatNames);
      }

      luuDuLieu();
      return;
    }
  }
  std::cout << "❌ Không tìm thấy đơn: " << maDon << "\n";
}

// ============================================================
// MENU ADMIN
// ============================================================
void menuAdmin() {
  if (!adminHienTai)
    return;

  while (true) {
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║      🛠️  BẢNG ĐIỀU KHIỂN ADMIN       ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  Xin chào, " << std::left << std::setw(26)
              << adminHienTai->getHoTen() << "║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  QUẢN LÝ PHIM                        ║\n";
    std::cout << "║  [1] Xem danh sách phim              ║\n";
    std::cout << "║  [2] Thêm phim mới                   ║\n";
    std::cout << "║  [3] Sửa thông tin phim              ║\n";
    std::cout << "║  [4] Xóa phim                        ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  QUẢN LÝ LỊCH CHIẾU                  ║\n";
    std::cout << "║  [5] Xem toàn bộ lịch chiếu          ║\n";
    std::cout << "║  [6] Thêm suất chiếu                 ║\n";
    std::cout << "║  [7] Hủy suất chiếu                  ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  THỐNG KÊ                            ║\n";
    std::cout << "║  [8] Thống kê doanh thu hôm nay      ║\n";
    std::cout << "║  [9] Thống kê theo phim              ║\n";
    std::cout << "║  [10] Xem thông tin rạp              ║\n";
    std::cout << "║  [0] Đăng xuất                       ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    int lua = nhapSoNguyen("Chọn: ");

    switch (lua) {
    case 1:
      menuXemPhim();
      break;
    case 2: {
      std::string ten = nhapChuoi("Tên phim: ");
      std::string loai = nhapChuoi("Thể loại: ");
      int tl = nhapSoNguyen("Thời lượng (phút): ");
      std::string moTa = nhapChuoi("Mô tả: ");
      int tuoi = nhapSoNguyen("Giới hạn tuổi (0/13/16/18): ");
      std::string ngay = nhapChuoi("Ngày khởi chiếu (YYYY-MM-DD): ");
      std::string maPhim =
          adminHienTai->themPhim(ten, loai, tl, moTa, tuoi, ngay);
      if (!maPhim.empty()) {
        Movie newPhim(maPhim, ten, loai, tl, ngay, moTa, tuoi);
        newPhim.setPoster("🎬");
        danhSachPhim.push_back(newPhim);
        lichChieu.themPhimVaoHeThong(&danhSachPhim.back());
        luuDuLieu();
      }
      break;
    }
    case 3: {
      std::string ma = nhapChuoi("Mã phim cần sửa: ");
      std::string tr = nhapChuoi("Trường cần sửa (tenPhim/moTa/theLoai): ");
      std::string gv = nhapChuoi("Giá trị mới: ");
      if (adminHienTai->suaPhim(ma, tr, gv)) {
        for (auto &phim : danhSachPhim) {
          if (phim.getMaPhim() == ma) {
            if (tr == "tenPhim")
              phim.setTenPhim(gv);
            else if (tr == "moTa")
              phim.setMoTa(gv);
            else if (tr == "theLoai")
              phim.setTheLoai(gv);
            break;
          }
        }
        luuDuLieu();
      }
      break;
    }
    case 4: {
      std::string ma = nhapChuoi("Mã phim cần xóa: ");
      if (adminHienTai->xoaPhim(ma)) {
        danhSachPhim.erase(
            std::remove_if(danhSachPhim.begin(), danhSachPhim.end(),
                           [&](const Movie &m) { return m.getMaPhim() == ma; }),
            danhSachPhim.end());
        // Xóa khỏi lịch chiếu
        lichChieu.getDanhSachPhimRef().erase(
            std::remove_if(
                lichChieu.getDanhSachPhimRef().begin(),
                lichChieu.getDanhSachPhimRef().end(),
                [&](const Movie *m) { return m->getMaPhim() == ma; }),
            lichChieu.getDanhSachPhimRef().end());
        luuDuLieu();
      }
      break;
    }
    case 5:
      lichChieu.hienThiToanBoLichChieu();
      break;
    case 6: {
      std::string maPhim = nhapChuoi("Mã phim: ");
      std::string maPhong = nhapChuoi("Mã phòng (P01/P02/P03/P04): ");
      std::string tgian = nhapChuoi("Giờ chiếu (YYYY-MM-DD HH:MM): ");
      double gia = static_cast<double>(nhapSoNguyen("Giá vé cơ sở (đồng): "));
      std::string nn = nhapChuoi("Ngôn ngữ (Phụ đề Việt/Thuyết minh): ");
      std::string maSuat =
          lichChieu.themBophimVaoLich(maPhim, maPhong, tgian, gia, nn);
      if (!maSuat.empty()) {
        luuDuLieu();
      }
      break;
    }
    case 7: {
      std::string maSuat = nhapChuoi("Mã suất chiếu cần hủy: ");
      if (adminHienTai->huyLichChieu(maSuat)) {
        lichChieu.xoaSuatChieu(maSuat);
        luuDuLieu();
      }
      break;
    }
    case 8: {
      time_t t = time(nullptr);
      tm *tmInfo = localtime(&t);
      char buf[11];
      strftime(buf, sizeof(buf), "%Y-%m-%d", tmInfo);
      adminHienTai->thongKeDoanhThuTheoNgay(std::string(buf));
      break;
    }
    case 9:
      adminHienTai->thongKeDoanhThuTheoPhim();
      break;
    case 10:
      rap.hienThiThongTin();
      rap.hienThiDanhSachPhong();
      break;
    case 0:
      adminHienTai->dangXuat();
      adminHienTai = nullptr;
      return;
    default:
      std::cout << "⚠️  Lựa chọn không hợp lệ!\n";
    }
  }
}

// ============================================================
// MENU KHÁCH HÀNG CHÍNH
// ============================================================
void menuKhachHang() {
  while (true) {
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║       🎬  MENU KHÁCH HÀNG            ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  Xin chào, " << std::left << std::setw(26)
              << khachHienTai->getHoTen() << "║\n";
    std::cout << "║  Hạng: " << std::left << std::setw(30)
              << khachHienTai->getHangThanhVien() << "║\n";
    std::cout << "║  Điểm: " << std::left << std::setw(30)
              << khachHienTai->getDiemTichLuy() << "║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  [1] Xem phim đang chiếu             ║\n";
    std::cout << "║  [2] Xem chi tiết phim               ║\n";
    std::cout << "║  [3] Xem lịch chiếu theo ngày        ║\n";
    std::cout << "║  [4] 🎟️  Đặt vé                      ║\n";
    std::cout << "║  [5] 📋  Lịch sử đặt vé              ║\n";
    std::cout << "║  [6] ❌  Hủy vé                      ║\n";
    std::cout << "║  [7] 👤  Thông tin tài khoản         ║\n";
    std::cout << "║  [0] Đăng xuất                       ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    int lua = nhapSoNguyen("Chọn: ");

    switch (lua) {
    case 1:
      menuXemPhim();
      break;
    case 2:
      menuXemChiTietPhim();
      break;
    case 3:
      menuXemLichChieu();
      break;
    case 4:
      menuDatVe();
      break;
    case 5:
      menuXemLichSu();
      break;
    case 6:
      menuHuyVe();
      break;
    case 7:
      khachHienTai->hienThiThongTin();
      break;
    case 0:
      khachHienTai->dangXuat();
      khachHienTai = nullptr;
      return;
    default:
      std::cout << "⚠️  Lựa chọn không hợp lệ!\n";
    }
  }
}

// ============================================================
// MENU CHÍNH
// ============================================================
void menuChinh() {
  while (true) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════╗\n";
    std::cout << "║   🎬  HỆ THỐNG ĐẶT VÉ XEM PHIM CINEMA    ║\n";
    std::cout << "║         CGV Vincom - Hà Nội               ║\n";
    std::cout << "╠════════════════════════════════════════════╣\n";
    std::cout << "║   [1]  Đăng nhập                          ║\n";
    std::cout << "║   [2]  Đăng ký tài khoản mới              ║\n";
    std::cout << "║   [3]  Xem phim đang chiếu (không cần đăng nhập) ║\n";
    std::cout << "║   [0]  Thoát                              ║\n";
    std::cout << "╚════════════════════════════════════════════╝\n";

    int lua = nhapSoNguyen("Chọn: ");

    switch (lua) {
    case 1:
      if (menuDangNhap()) {
        if (adminHienTai)
          menuAdmin();
        if (khachHienTai)
          menuKhachHang();
      }
      break;
    case 2:
      menuDangKy();
      break;
    case 3:
      menuXemPhim();
      break;
    case 0:
      std::cout << "\n👋 Cảm ơn bạn đã sử dụng dịch vụ Cinema!\n";
      std::cout << "   Hẹn gặp lại! 🎬\n\n";
      return;
    default:
      std::cout << "⚠️  Vui lòng chọn 0-3!\n";
    }
  }
}

// ============================================================
// XỬ LÝ API CHO WEB INTERFACE
// ============================================================
void processAPI(int argc, char *argv[]) {
  std::string command = argv[1];

  // Log chi tiết ra terminal để bạn dễ theo dõi khi đang thao tác trên Web
  // (Dùng std::cerr để trình duyệt Web không bị nhận nhầm các dòng chữ này)
  std::cerr << "\n[🌐 WEB API] Đang xử lý Request: " << command;
  if (argc > 2) {
    std::cerr << " | Tham số: ";
    for (int i = 2; i < argc; ++i)
      std::cerr << argv[i] << " ";
  }
  std::cerr << std::endl;

  // Đọc dữ liệu từ đĩa trong yên lặng để tránh rác stdout
  std::streambuf *old_cout = std::cout.rdbuf();
  std::stringstream ss;
  std::cout.rdbuf(ss.rdbuf());

  khoiTaoDuLieuMau();

  std::cout.rdbuf(old_cout); // Phục hồi stdout

  if (command == "--api-get-movies") {
    std::cout << "[";
    for (size_t i = 0; i < danhSachPhim.size(); ++i) {
      const auto &m = danhSachPhim[i];
      std::cout << "{"
                << "\"id\":\"" << m.getMaPhim() << "\","
                << "\"title\":\"" << m.getTenPhim() << "\","
                << "\"titleOrig\":\"" << m.getTenPhimGoc() << "\","
                << "\"genre\":\"" << m.getTheLoai() << "\","
                << "\"duration\":" << m.getThoiLuong() << ","
                << "\"rating\":" << m.getDiemDanhGia() << ","
                << "\"votes\":" << m.getSoLuotDanhGia() << ","
                << "\"ageLimit\":" << m.getGioiHanTuoi() << ","
                << "\"director\":\"" << m.getDaoDien() << "\","
                << "\"cast\":\"" << m.getDienVienChinh() << "\","
                << "\"desc\":\"" << m.getMoTa() << "\","
                << "\"poster\":\"" << m.getPoster() << "\","
                << "\"status\":\"" << (m.isDangChieu() ? "hot" : "") << "\","
                << "\"releaseDate\":\"" << m.getNgayKhoiChieu() << "\""
                << "}" << (i + 1 < danhSachPhim.size() ? "," : "");
    }
    std::cout << "]" << std::endl;
    return;
  }

  if (command == "--api-get-showtimes") {
    std::cout << "[";
    auto suats = lichChieu.getDanhSachSuatChieu();
    for (size_t i = 0; i < suats.size(); ++i) {
      const auto &s = suats[i];
      std::string fullTime = s.getThoiGianBatDau();
      std::string date = fullTime.substr(0, 10);
      std::string time = fullTime.substr(11);
      int availableSeats =
          s.getPhongChieu() ? s.getPhongChieu()->demGheTrong() : 0;

      std::cout << "{"
                << "\"id\":\"" << s.getMaSuatChieu() << "\","
                << "\"movieId\":\""
                << (s.getPhim() ? s.getPhim()->getMaPhim() : "") << "\","
                << "\"roomId\":\""
                << (s.getPhongChieu() ? s.getPhongChieu()->getMaPhong() : "")
                << "\","
                << "\"date\":\"" << date << "\","
                << "\"time\":\"" << time << "\","
                << "\"lang\":\"" << s.getNgonNguPhim() << "\","
                << "\"special\":" << (s.isLaSuatDacBiet() ? "true" : "false")
                << ","
                << "\"price\":" << s.getGiaVeCoSo() << ","
                << "\"status\":\"" << (s.coTheDatVe() ? "selling" : "full")
                << "\","
                << "\"availableSeats\":" << availableSeats << "}"
                << (i + 1 < suats.size() ? "," : "");
    }
    std::cout << "]" << std::endl;
    return;
  }

  if (command == "--api-get-rooms") {
    std::cout << "[";
    auto rooms = rap.getDanhSachPhong();
    for (size_t i = 0; i < rooms.size(); ++i) {
      const auto &r = rooms[i];
      std::cout << "{"
                << "\"id\":\"" << r.getMaPhong() << "\","
                << "\"name\":\"" << r.getTenPhong() << "\","
                << "\"type\":\"" << Room::loaiPhongToString(r.getLoaiPhong())
                << "\","
                << "\"rows\":" << r.getSoHang() << ","
                << "\"seatsPerRow\":" << r.getSoGheMoiHang() << ","
                << "\"basePrice\":"
                << (r.getDanhSachGhe().empty()
                        ? 80000.0
                        : r.getDanhSachGhe()[0].getGiaGhe())
                << "}" << (i + 1 < rooms.size() ? "," : "");
    }
    std::cout << "]" << std::endl;
    return;
  }

  if (command == "--api-get-sold-seats" && argc >= 3) {
    std::string showtimeId = argv[2];
    Showtime *suat = lichChieu.timSuatChieuBangMa(showtimeId);
    std::cout << "[";
    if (suat && suat->getPhongChieu()) {
      bool first = true;
      for (const auto &ghe : suat->getPhongChieu()->getDanhSachGhe()) {
        if (!ghe.coTheDat()) {
          if (!first)
            std::cout << ",";
          first = false;
          std::cout << "\"" << ghe.getMaGhe() << "\"";
        }
      }
    }
    std::cout << "]" << std::endl;
    return;
  }

  if (command == "--api-login" && argc >= 4) {
    std::string email = argv[2];
    std::string password = argv[3];

    bool found = false;
    for (auto &admin : danhSachAdmin) {
      if (admin.getEmail() == email && admin.kiemTraMatKhau(password)) {
        std::cout << "{\"ok\":true,\"user\":{"
                  << "\"id\":" << admin.getId() << ","
                  << "\"email\":\"" << admin.getEmail() << "\","
                  << "\"name\":\"" << admin.getHoTen() << "\","
                  << "\"phone\":\"" << admin.getSoDienThoai() << "\","
                  << "\"role\":\"admin\","
                  << "\"points\":0,"
                  << "\"totalSpent\":0,"
                  << "\"tier\":\"Gold\""
                  << "}}" << std::endl;
        found = true;
        break;
      }
    }
    if (!found) {
      for (auto &kh : danhSachKhachHang) {
        if (kh.getEmail() == email && kh.kiemTraMatKhau(password)) {
          std::cout << "{\"ok\":true,\"user\":{"
                    << "\"id\":" << kh.getId() << ","
                    << "\"email\":\"" << kh.getEmail() << "\","
                    << "\"name\":\"" << kh.getHoTen() << "\","
                    << "\"phone\":\"" << kh.getSoDienThoai() << "\","
                    << "\"role\":\"customer\","
                    << "\"points\":" << kh.getDiemTichLuy() << ","
                    << "\"totalSpent\":" << kh.getTongTienDaMua() << ","
                    << "\"tier\":\"" << kh.getHangThanhVien() << "\""
                    << "}}" << std::endl;
          found = true;
          break;
        }
      }
    }
    if (!found) {
      std::cout << "{\"ok\":false,\"msg\":\"Sai email hoặc mật khẩu!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-register" && argc >= 6) {
    std::string name = argv[2];
    std::string email = argv[3];
    std::string phone = argv[4];
    std::string password = argv[5];

    bool exists = false;
    for (const auto &kh : danhSachKhachHang) {
      if (kh.getEmail() == email) {
        exists = true;
        break;
      }
    }
    if (exists) {
      std::cout << "{\"ok\":false,\"msg\":\"Email này đã được đăng ký!\"}"
                << std::endl;
      return;
    }

    int idMoi = 200 + (int)danhSachKhachHang.size() + 1;
    danhSachKhachHang.emplace_back(idMoi, name, email, phone, password);
    luuDuLieu();

    const auto &c = danhSachKhachHang.back();
    std::cout << "{\"ok\":true,\"user\":{"
              << "\"id\":" << c.getId() << ","
              << "\"email\":\"" << c.getEmail() << "\","
              << "\"name\":\"" << c.getHoTen() << "\","
              << "\"phone\":\"" << c.getSoDienThoai() << "\","
              << "\"role\":\"customer\","
              << "\"points\":" << c.getDiemTichLuy() << ","
              << "\"totalSpent\":" << c.getTongTienDaMua() << ","
              << "\"tier\":\"" << c.getHangThanhVien() << "\""
              << "}}" << std::endl;
    return;
  }

  if (command == "--api-book" && argc >= 7) {
    int userId = std::stoi(argv[2]);
    std::string showtimeId = argv[3];
    std::string seatsStr = argv[4];
    int payMethodVal = std::stoi(argv[5]);
    int pointsUsed = std::stoi(argv[6]);
    double comboPrice = 0.0;
    if (argc >= 8) {
      try { comboPrice = std::stod(argv[7]); } catch (...) {}
    }

    Customer *kh = nullptr;
    for (auto &c : danhSachKhachHang) {
      if (c.getId() == userId) {
        kh = &c;
        break;
      }
    }
    Showtime *suat = lichChieu.timSuatChieuBangMa(showtimeId);
    if (!kh || !suat) {
      std::cout << "{\"ok\":false,\"msg\":\"Dữ liệu người dùng hoặc suất chiếu "
                   "không hợp lệ!\"}"
                << std::endl;
      return;
    }

    std::vector<std::string> seatsList;
    std::stringstream ss(seatsStr);
    std::string seat;
    while (std::getline(ss, seat, ',')) {
      if (!seat.empty())
        seatsList.push_back(seat);
    }

    Room *phong = suat->getPhongChieu();
    if (!phong) {
      std::cout << "{\"ok\":false,\"msg\":\"Phòng chiếu không hợp lệ!\"}"
                << std::endl;
      return;
    }
    for (const auto &sName : seatsList) {
      Seat *g = phong->timGheBangMa(sName);
      if (!g || !g->coTheDat()) {
        std::cout << "{\"ok\":false,\"msg\":\"Ghế " << sName
                  << " đã bị đặt hoặc không tồn tại!\"}" << std::endl;
        return;
      }
    }

    PhuongThucThanhToan pttt = static_cast<PhuongThucThanhToan>(payMethodVal);
    Booking donMoi(kh, pttt);
    std::vector<Ticket> tickets;
    static int ticketSeq = (int)danhSachDon.size() * 5 + 1;

    for (const auto &sName : seatsList) {
      Seat *g = phong->timGheBangMa(sName);
      std::string maVe = Ticket::taoMaVeMoi(ticketSeq++);
      Ticket ve(maVe, suat, g, kh->getHoTen(), kh->getEmail(),
                donMoi.getMaDon());
      ve.tinhGiaVe();
      ve.taoMaQR();
      tickets.push_back(ve);
    }

    donMoi.setTienBapNuoc(comboPrice);
    donMoi.taoDonDat(tickets);
    donMoi.tinhTongTien();

    if (pointsUsed > 0 && kh->getDiemTichLuy() >= pointsUsed) {
      double discount = (pointsUsed / 100.0) * 50000.0;
      kh->doiDiem(pointsUsed);
      donMoi.apDungGiamGia(discount);
    }

    phong->datNhieuGhe(seatsList, donMoi.getMaDon());

    if (paymentSvc.xuLyThanhToan(donMoi, pttt)) {
      danhSachDon.push_back(donMoi);
      luuDuLieu();

      std::cout << "{\"ok\":true,"
                << "\"bookingCode\":\"" << donMoi.getMaDatChoNgauNhien()
                << "\","
                << "\"bookingId\":\"" << donMoi.getMaDon() << "\","
                << "\"total\":" << donMoi.getSoTienThucTe() << ","
                << "\"points\":" << kh->getDiemTichLuy() << "}" << std::endl;
    } else {
      phong->huyNhieuGhe(seatsList);
      std::cout << "{\"ok\":false,\"msg\":\"Thanh toán thất bại!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-get-tickets" && argc >= 3) {
    int userId = std::stoi(argv[2]);
    std::cout << "[";
    bool first = true;
    for (const auto &b : danhSachDon) {
      if (userId == 0 ||
          (b.getKhachHang() && b.getKhachHang()->getId() == userId)) {
        if (!first)
          std::cout << ",";
        first = false;

        std::cout
            << "{"
            << "\"id\":\"" << b.getMaDon() << "\","
            << "\"bookingCode\":\"" << b.getMaDatChoNgauNhien() << "\","
            << "\"userName\":\""
            << (b.getKhachHang() ? b.getKhachHang()->getHoTen() : "") << "\","
            << "\"payMethod\":\""
            << Booking::ptttToString(b.getPhuongThucThanhToan()) << "\","
            << "\"movieTitle\":\"";
        {
          const auto& veList = b.getDanhSachVe();
          Showtime* st0 = (!veList.empty()) ? veList[0].getSuatChieu() : nullptr;
          Movie*    mv0 = (st0) ? st0->getPhim() : nullptr;
          Room*     rm0 = (st0) ? st0->getPhongChieu() : nullptr;
          std::cout
            << (mv0 ? mv0->getTenPhim() : "") << "\","
            << "\"movieId\":\"" << (mv0 ? mv0->getMaPhim() : "") << "\","
            << "\"showtimeId\":\"" << (st0 ? st0->getMaSuatChieu() : "") << "\","
            << "\"roomName\":\"" << (rm0 ? rm0->getTenPhong() : "") << "\","
            << "\"roomId\":\"" << (rm0 ? rm0->getMaPhong() : "") << "\","
            << "\"showDate\":\"" << (st0 ? st0->getNgayChieu() : "") << "\","
            << "\"showTime\":\"";
          std::string tgbd = st0 ? st0->getThoiGianBatDau() : "";
          std::cout << (tgbd.size() > 11 ? tgbd.substr(11) : "") << "\",";
        }
        std::cout
            << "\"total\":" << b.getSoTienThucTe() << ","
            << "\"status\":\"" << Booking::trangThaiToString(b.getTrangThai())
            << "\","
            << "\"createdAt\":\"" << b.getNgayDat() << "\","
            << "\"seats\":\"";
        for (size_t k = 0; k < b.getDanhSachVe().size(); ++k) {
          std::cout << (b.getDanhSachVe()[k].getGhe() ? b.getDanhSachVe()[k].getGhe()->getMaGhe() : "")
                    << (k + 1 < b.getDanhSachVe().size() ? ", " : "");
        }
        std::cout << "\",\"tickets\":[";
        for (size_t k = 0; k < b.getDanhSachVe().size(); ++k) {
          const auto &t = b.getDanhSachVe()[k];
          std::cout << "{"
                    << "\"id\":\"" << t.getMaVe() << "\","
                    << "\"seatId\":\"" << (t.getGhe() ? t.getGhe()->getMaGhe() : "") << "\","
                    << "\"price\":" << t.getGiaVe() << ","
                    << "\"qrCode\":\"" << t.getMaQR() << "\","
                    << "\"status\":\""
                    << Ticket::trangThaiToString(t.getTrangThai()) << "\""
                    << "}" << (k + 1 < b.getDanhSachVe().size() ? "," : "");
        }
        std::cout << "]}";
      }
    }
    std::cout << "]" << std::endl;
    return;
  }

  if (command == "--api-cancel-ticket" && argc >= 3) {
    std::string bookingId = argv[2];
    for (auto &b : danhSachDon) {
      if (b.getMaDon() == bookingId) {
        if (b.getTrangThai() == TrangThaiDon::DA_HUY ||
            b.getTrangThai() == TrangThaiDon::DA_HOAN_TIEN) {
          std::cout
              << "{\"ok\":false,\"msg\":\"Đơn hàng đã được hủy trước đó!\"}"
              << std::endl;
          return;
        }
        if (!b.coTheHuy()) {
          std::cout << "{\"ok\":false,\"msg\":\"Đơn hàng này không thể hủy (đã "
                       "sát giờ chiếu)!\"}"
                    << std::endl;
          return;
        }

        double hoursLeft = 30.0;
        double refundAmount = paymentSvc.xuLyHoanTien(b, hoursLeft);
        b.huyDon();

        std::vector<std::string> seatNames;
        for (const auto &t : b.getDanhSachVe()) {
          if (t.getGhe())
            seatNames.push_back(t.getGhe()->getMaGhe());
        }
        if (b.getDanhSachVe().empty() == false &&
            b.getDanhSachVe()[0].getSuatChieu() &&
            b.getDanhSachVe()[0].getSuatChieu()->getPhongChieu()) {
          b.getDanhSachVe()[0].getSuatChieu()->getPhongChieu()->huyNhieuGhe(
              seatNames);
        }

        luuDuLieu();
        std::cout << "{\"ok\":true,\"refundAmount\":" << refundAmount << "}"
                  << std::endl;
        return;
      }
    }
    std::cout << "{\"ok\":false,\"msg\":\"Không tìm thấy đơn hàng!\"}"
              << std::endl;
    return;
  }

  if (command == "--api-get-dashboard") {
    double totalRevenue = 0;
    int totalTickets = 0;
    for (const auto &b : danhSachDon) {
      if (b.getTrangThai() == TrangThaiDon::DA_THANH_TOAN) {
        totalRevenue += b.getSoTienThucTe();
        totalTickets += b.getSoVe();
      }
    }
    int activeMovies = 0;
    for (const auto &m : danhSachPhim) {
      if (m.isDangChieu())
        activeMovies++;
    }
    int totalCustomers = (int)danhSachKhachHang.size();

    std::cout << "{"
              << "\"totalRevenue\":" << totalRevenue << ","
              << "\"ticketsSold\":" << totalTickets << ","
              << "\"activeMovies\":" << activeMovies << ","
              << "\"totalCustomers\":" << totalCustomers << "}" << std::endl;
    return;
  }

  if (command == "--api-add-movie" && argc >= 9) {
    std::string ten = argv[2];
    std::string theloai = argv[3];
    int thoiluong = std::stoi(argv[4]);
    std::string mota = argv[5];
    int tuoi = std::stoi(argv[6]);
    std::string ngay = argv[7];
    std::string poster = argv[8];

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1000, 9999);
    std::string maPhim = "MV" + std::to_string(dist(rng));

    Movie newPhim(maPhim, ten, theloai, thoiluong, ngay, mota, tuoi);
    newPhim.setPoster(poster);
    danhSachPhim.push_back(newPhim);
    lichChieu.themPhimVaoHeThong(&danhSachPhim.back());
    luuDuLieu();

    std::cout << "{\"ok\":true,\"maPhim\":\"" << maPhim << "\"}" << std::endl;
    return;
  }

  if (command == "--api-add-showtime" && argc >= 7) {
    std::string maphim = argv[2];
    std::string maphong = argv[3];
    std::string thoigian = argv[4];
    double gia = std::stod(argv[5]);
    std::string ngonngu = argv[6];

    std::string maSuat =
        lichChieu.themBophimVaoLich(maphim, maphong, thoigian, gia, ngonngu);
    if (!maSuat.empty()) {
      luuDuLieu();
      std::cout << "{\"ok\":true,\"maSuat\":\"" << maSuat << "\"}" << std::endl;
    } else {
      std::cout << "{\"ok\":false,\"msg\":\"Trùng lịch chiếu hoặc dữ liệu "
                   "không hợp lệ!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-delete-showtime" && argc >= 3) {
    std::string maSuat = argv[2];
    if (lichChieu.xoaSuatChieu(maSuat)) {
      luuDuLieu();
      std::cout << "{\"ok\":true}" << std::endl;
    } else {
      std::cout << "{\"ok\":false,\"msg\":\"Không tìm thấy suất chiếu!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-delete-movie" && argc >= 3) {
    std::string ma = argv[2];
    bool found = false;
    for (const auto &phim : danhSachPhim) {
      if (phim.getMaPhim() == ma) {
        found = true;
        break;
      }
    }
    if (found) {
      danhSachPhim.erase(
          std::remove_if(danhSachPhim.begin(), danhSachPhim.end(),
                         [&](const Movie &m) { return m.getMaPhim() == ma; }),
          danhSachPhim.end());

      // Xóa khỏi lịch chiếu
      lichChieu.getDanhSachPhimRef().erase(
          std::remove_if(
              lichChieu.getDanhSachPhimRef().begin(),
              lichChieu.getDanhSachPhimRef().end(),
              [&](const Movie *m) { return m && m->getMaPhim() == ma; }),
          lichChieu.getDanhSachPhimRef().end());

      // Xóa tất cả các suất chiếu liên quan
      std::vector<std::string> toDelete;
      for (const auto &s : lichChieu.getDanhSachSuatChieu()) {
        if (s.getPhim() && s.getPhim()->getMaPhim() == ma) {
          toDelete.push_back(s.getMaSuatChieu());
        }
      }
      for (const auto &sid : toDelete) {
        lichChieu.xoaSuatChieu(sid);
      }

      luuDuLieu();
      std::cout << "{\"ok\":true}" << std::endl;
    } else {
      std::cout << "{\"ok\":false,\"msg\":\"Không tìm thấy phim!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-update-movie" && argc >= 10) {
    std::string ma = argv[2];
    std::string ten = argv[3];
    std::string theloai = argv[4];
    int thoiluong = std::stoi(argv[5]);
    std::string mota = argv[6];
    int tuoi = std::stoi(argv[7]);
    std::string ngay = argv[8];
    std::string poster = argv[9];

    bool found = false;
    for (auto &phim : danhSachPhim) {
      if (phim.getMaPhim() == ma) {
        phim.setTenPhim(ten);
        phim.setTheLoai(theloai);
        phim.setThoiLuong(thoiluong);
        phim.setMoTa(mota);
        phim.setGioiHanTuoi(tuoi);
        phim.setNgayKhoiChieu(ngay);
        phim.setPoster(poster);
        found = true;
        break;
      }
    }
    if (found) {
      luuDuLieu();
      std::cout << "{\"ok\":true}" << std::endl;
    } else {
      std::cout << "{\"ok\":false,\"msg\":\"Không tìm thấy phim!\"}"
                << std::endl;
    }
    return;
  }

  if (command == "--api-get-customers") {
    std::cout << "[";
    for (size_t i = 0; i < danhSachKhachHang.size(); ++i) {
      const auto &kh = danhSachKhachHang[i];
      std::cout << "{"
                << "\"id\":" << kh.getId() << ","
                << "\"name\":\"" << kh.getHoTen() << "\","
                << "\"email\":\"" << kh.getEmail() << "\","
                << "\"phone\":\"" << kh.getSoDienThoai() << "\","
                << "\"points\":" << kh.getDiemTichLuy() << ","
                << "\"totalSpent\":" << kh.getTongTienDaMua() << ","
                << "\"tier\":\"" << kh.getHangThanhVien() << "\""
                << "}" << (i + 1 < danhSachKhachHang.size() ? "," : "");
    }
    std::cout << "]" << std::endl;

    std::cerr << "[✅ WEB API] Đã gửi danh sách khách hàng về Web thành công!"
              << std::endl;
    return;
  }

  std::cout << "{\"ok\":false,\"msg\":\"Lệnh API không hợp lệ!\"}" << std::endl;
  std::cerr << "[❌ WEB API] Lỗi: Không nhận diện được lệnh API!" << std::endl;
}

// ============================================================
// HÀM MAIN - Điểm khởi đầu chương trình
// ============================================================
int main(int argc, char *argv[]) {
#ifdef _WIN32
  system("chcp 65001 > nul");
  int argc_utf8;
  LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_utf8);
  std::vector<std::string> args(argc_utf8);
  for (int i = 0; i < argc_utf8; ++i) {
    int size = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, &utf8Str[0], size, nullptr, nullptr);
    utf8Str.resize(size - 1);
    args[i] = utf8Str;
  }
  LocalFree(argv_w);
  std::vector<char *> utf8_argv(argc_utf8);
  for (int i = 0; i < argc_utf8; ++i) utf8_argv[i] = &args[i][0];
  argc = argc_utf8;
  argv = utf8_argv.data();
#endif

  if (argc > 1) {
    processAPI(argc, argv);
    return 0;
  }

  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║   🎬   CINEMA BOOKING SYSTEM - PHIÊN BẢN 1.0   🎬      ║\n";
  std::cout << "║         Hệ Thống Đặt Vé Xem Phim Hiện Đại              ║\n";
  std::cout << "║   Tính năng: QR Code | Hoàn tiền | Điểm tích lũy       ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";

  // Khởi tạo dữ liệu mẫu
  khoiTaoDuLieuMau();

  // Hiển thị gợi ý đăng nhập
  std::cout << "💡 Tài khoản demo:\n";
  std::cout << "   Admin : admin@cinema.vn / admin123\n";
  std::cout << "   Khách : an.tran@gmail.com / 123456\n\n";

  // Chạy menu chính
  menuChinh();

  return 0;
}
