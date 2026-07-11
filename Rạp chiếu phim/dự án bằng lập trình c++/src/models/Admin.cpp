#include "Admin.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <random>

// ============================================================
// Admin.cpp - Triển khai lớp Quản trị viên
// ============================================================

// Chuyển enum QuyenHan sang chuỗi để hiển thị
std::string Admin::quyenHanToString(QuyenHan quyen) {
    switch (quyen) {
        case QuyenHan::STAFF:       return "Nhân viên (STAFF)";
        case QuyenHan::MANAGER:     return "Quản lý (MANAGER)";
        case QuyenHan::SUPER_ADMIN: return "Siêu quản trị (SUPER_ADMIN)";
        default:                    return "Không xác định";
    }
}

// ---- Constructor mặc định ----
Admin::Admin()
    : User(), quyenHan(QuyenHan::STAFF),
      phongBan("Vận hành"), maNhanVien("NV000") {
    vaiTro = "admin";
}

// ---- Constructor có tham số ----
Admin::Admin(int id, const std::string& hoTen, const std::string& email,
             const std::string& soDienThoai, const std::string& matKhau,
             QuyenHan quyenHan, const std::string& phongBan)
    : User(id, hoTen, email, soDienThoai, matKhau, "admin"),
      quyenHan(quyenHan), phongBan(phongBan) {
    // Tạo mã nhân viên: NV + id (ví dụ: NV001)
    std::ostringstream oss;
    oss << "NV" << std::setw(3) << std::setfill('0') << id;
    maNhanVien = oss.str();
}

Admin::~Admin() {}

// ============================================================
// Kiểm tra quyền hạn
// ============================================================
bool Admin::kiemTraQuyen(QuyenHan quyenYeuCau) const {
    // So sánh mức quyền (giá trị enum cao hơn = quyền cao hơn)
    return static_cast<int>(quyenHan) >= static_cast<int>(quyenYeuCau);
}

// ============================================================
// QUẢN LÝ PHIM
// ============================================================

std::string Admin::themPhim(const std::string& tenPhim,
                             const std::string& theLoai,
                             int thoiLuong,
                             const std::string& moTa,
                             int gioiHanTuoi,
                             const std::string& ngayKhoiChieu) {
    // Kiểm tra quyền
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền thêm phim! Cần quyền MANAGER.\n";
        return "";
    }

    // Validate dữ liệu đầu vào
    if (tenPhim.empty()) {
        std::cout << "❌ Tên phim không được để trống!\n";
        return "";
    }
    if (thoiLuong <= 0 || thoiLuong > 300) {
        std::cout << "❌ Thời lượng phim không hợp lệ (1-300 phút)!\n";
        return "";
    }

    // Tạo mã phim ngẫu nhiên
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1000, 9999);
    std::string maPhim = "MV" + std::to_string(dist(rng));

    std::cout << "✅ Thêm phim thành công!\n";
    std::cout << "   Mã phim   : " << maPhim       << "\n";
    std::cout << "   Tên phim  : " << tenPhim      << "\n";
    std::cout << "   Thể loại  : " << theLoai      << "\n";
    std::cout << "   Thời lượng: " << thoiLuong    << " phút\n";
    std::cout << "   Giới hạn  : " << gioiHanTuoi  << "+\n";
    std::cout << "   Khởi chiếu: " << ngayKhoiChieu << "\n";

    return maPhim;
}

bool Admin::suaPhim(const std::string& maPhim,
                    const std::string& truong,
                    const std::string& giaTri) {
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền sửa phim!\n";
        return false;
    }

    std::cout << "✅ Đã cập nhật phim " << maPhim
              << " | " << truong << " → " << giaTri << "\n";
    return true;
}

bool Admin::xoaPhim(const std::string& maPhim) {
    if (!kiemTraQuyen(QuyenHan::SUPER_ADMIN)) {
        std::cout << "❌ Chỉ SUPER_ADMIN mới có quyền xóa phim!\n";
        return false;
    }

    std::cout << "⚠️  Bạn có chắc muốn xóa phim " << maPhim << "? (y/n): ";
    char xacNhan;
    std::cin >> xacNhan;
    if (xacNhan == 'y' || xacNhan == 'Y') {
        std::cout << "✅ Đã xóa phim: " << maPhim << "\n";
        return true;
    }
    std::cout << "↩️  Đã hủy thao tác xóa.\n";
    return false;
}

// ============================================================
// QUẢN LÝ LỊCH CHIẾU
// ============================================================

std::string Admin::themLichChieu(const std::string& maPhim,
                                  const std::string& maPhong,
                                  const std::string& thoiGianBatDau,
                                  double giaVeCoSo) {
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền thêm lịch chiếu!\n";
        return "";
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(10000, 99999);
    std::string maSuatChieu = "ST" + std::to_string(dist(rng));

    std::cout << "✅ Thêm lịch chiếu thành công!\n";
    std::cout << "   Mã suất   : " << maSuatChieu    << "\n";
    std::cout << "   Phim      : " << maPhim         << "\n";
    std::cout << "   Phòng     : " << maPhong        << "\n";
    std::cout << "   Giờ chiếu : " << thoiGianBatDau << "\n";
    std::cout << "   Giá cơ sở : " << std::fixed << std::setprecision(0)
              << giaVeCoSo << "đ\n";

    return maSuatChieu;
}

bool Admin::suaLichChieu(const std::string& maSuatChieu,
                          const std::string& truong,
                          const std::string& giaTri) {
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền sửa lịch chiếu!\n";
        return false;
    }
    std::cout << "✅ Đã cập nhật suất chiếu " << maSuatChieu
              << " | " << truong << " → " << giaTri << "\n";
    return true;
}

bool Admin::huyLichChieu(const std::string& maSuatChieu) {
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền hủy lịch chiếu!\n";
        return false;
    }
    std::cout << "⚠️  Hủy suất chiếu " << maSuatChieu
              << " - Sẽ hoàn tiền cho tất cả khách đã đặt.\n";
    std::cout << "✅ Đã hủy suất chiếu: " << maSuatChieu << "\n";
    return true;
}

// ============================================================
// THỐNG KÊ DOANH THU
// ============================================================

void Admin::thongKeDoanhThuTheoNgay(const std::string& ngay) const {
    std::cout << "\n📊 THỐNG KÊ DOANH THU NGÀY: " << ngay << "\n";
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  [Dữ liệu sẽ được tải từ ReportService]\n";
    std::cout << "══════════════════════════════════════════════\n";
}

void Admin::thongKeDoanhThuTheoThang(int thang, int nam) const {
    std::cout << "\n📊 THỐNG KÊ DOANH THU THÁNG " << thang << "/" << nam << "\n";
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  [Dữ liệu sẽ được tải từ ReportService]\n";
    std::cout << "══════════════════════════════════════════════\n";
}

void Admin::thongKeDoanhThuTheoPhim(const std::string& maPhim) const {
    std::cout << "\n📊 THỐNG KÊ DOANH THU THEO PHIM";
    if (!maPhim.empty()) std::cout << ": " << maPhim;
    std::cout << "\n══════════════════════════════════════════════\n";
    std::cout << "  [Dữ liệu sẽ được tải từ ReportService]\n";
    std::cout << "══════════════════════════════════════════════\n";
}

void Admin::thongKeTongQuan() const {
    std::cout << "\n📈 DASHBOARD - TỔNG QUAN HỆ THỐNG\n";
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  Ngày hôm nay   : [Từ hệ thống]\n";
    std::cout << "  Tổng doanh thu : [Từ ReportService]\n";
    std::cout << "  Vé đã bán      : [Từ BookingService]\n";
    std::cout << "  Phim đang chiếu: [Từ MovieService]\n";
    std::cout << "  Khách hàng mới : [Từ AuthService]\n";
    std::cout << "══════════════════════════════════════════════\n";
}

void Admin::xemDanhSachNguoiDung() const {
    if (!kiemTraQuyen(QuyenHan::SUPER_ADMIN)) {
        std::cout << "❌ Chỉ SUPER_ADMIN mới xem được danh sách người dùng!\n";
        return;
    }
    std::cout << "\n👥 DANH SÁCH NGƯỜI DÙNG\n";
    std::cout << "  [Dữ liệu sẽ được tải từ FileManager]\n";
}

bool Admin::khoaTaiKhoan(int userId) {
    if (!kiemTraQuyen(QuyenHan::MANAGER)) {
        std::cout << "❌ Bạn không có quyền khóa tài khoản!\n";
        return false;
    }
    std::cout << "🔒 Đã khóa tài khoản ID: " << userId << "\n";
    return true;
}

// ============================================================
// Hiển thị thông tin admin (Override)
// ============================================================
void Admin::hienThiThongTin() const {
    User::hienThiThongTin();
    std::cout << "  Mã NV      : " << maNhanVien                       << "\n";
    std::cout << "  Phòng ban  : " << phongBan                         << "\n";
    std::cout << "  Quyền hạn  : " << quyenHanToString(quyenHan)       << "\n";
    std::cout << "========================================\n";
}
