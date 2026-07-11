#include "User.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>   // std::hash

// ============================================================
// User.cpp - Triển khai lớp người dùng cơ sở
// ============================================================

// ---- Constructor mặc định ----
User::User() : id(0), hoTen(""), email(""), soDienThoai(""),
               matKhau(""), vaiTro("customer"), dangOnline(false) {}

// ---- Constructor có tham số ----
User::User(int id, const std::string& hoTen, const std::string& email,
           const std::string& soDienThoai, const std::string& matKhau,
           const std::string& vaiTro)
    : id(id), hoTen(hoTen), email(email), soDienThoai(soDienThoai),
      matKhau(maHoaMatKhau(matKhau)), vaiTro(vaiTro), dangOnline(false) {}

// ---- Destructor ----
User::~User() {}

// ============================================================
// Mã hóa mật khẩu đơn giản (dùng std::hash + muối)
// Trong hệ thống thực tế nên dùng bcrypt/argon2
// ============================================================
std::string User::maHoaMatKhau(const std::string& matKhau) {
    // Thêm "muối" vào mật khẩu trước khi hash
    std::string matKhauCoMuoi = "CinemaBooking_Salt_2024_" + matKhau;
    
    std::size_t hashValue = std::hash<std::string>{}(matKhauCoMuoi);
    
    // Chuyển số hash sang chuỗi hex
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hashValue;
    return oss.str();
}

// ============================================================
// Kiểm tra mật khẩu có khớp với mật khẩu đã lưu không
// ============================================================
bool User::kiemTraMatKhau(const std::string& matKhauNhap) const {
    return matKhau == maHoaMatKhau(matKhauNhap);
}

// ============================================================
// Đăng nhập: kiểm tra email + mật khẩu
// ============================================================
bool User::dangNhap(const std::string& emailInput,
                    const std::string& matKhauInput) {
    if (email == emailInput && kiemTraMatKhau(matKhauInput)) {
        dangOnline = true;
        std::cout << "✅ Đăng nhập thành công! Chào mừng, " << hoTen << "!\n";
        return true;
    }
    std::cout << "❌ Email hoặc mật khẩu không đúng. Vui lòng thử lại.\n";
    return false;
}

// ============================================================
// Đăng xuất
// ============================================================
void User::dangXuat() {
    dangOnline = false;
    std::cout << "👋 Tạm biệt, " << hoTen << "! Đã đăng xuất thành công.\n";
}

// ============================================================
// Cập nhật thông tin cá nhân
// ============================================================
void User::capNhatThongTin(const std::string& hoTenMoi,
                            const std::string& sdtMoi) {
    if (!hoTenMoi.empty()) {
        hoTen = hoTenMoi;
        std::cout << "✅ Đã cập nhật họ tên: " << hoTen << "\n";
    }
    if (!sdtMoi.empty()) {
        soDienThoai = sdtMoi;
        std::cout << "✅ Đã cập nhật số điện thoại: " << soDienThoai << "\n";
    }
}

// ============================================================
// Đổi mật khẩu
// ============================================================
bool User::doiMatKhau(const std::string& matKhauCu,
                       const std::string& matKhauMoi) {
    if (!kiemTraMatKhau(matKhauCu)) {
        std::cout << "❌ Mật khẩu cũ không đúng!\n";
        return false;
    }
    if (matKhauMoi.length() < 6) {
        std::cout << "❌ Mật khẩu mới phải có ít nhất 6 ký tự!\n";
        return false;
    }
    matKhau = maHoaMatKhau(matKhauMoi);
    std::cout << "✅ Đổi mật khẩu thành công!\n";
    return true;
}

// ============================================================
// Hiển thị thông tin người dùng
// ============================================================
void User::hienThiThongTin() const {
    std::cout << "========================================\n";
    std::cout << "  THÔNG TIN NGƯỜI DÙNG\n";
    std::cout << "========================================\n";
    std::cout << "  ID        : " << id           << "\n";
    std::cout << "  Họ tên    : " << hoTen        << "\n";
    std::cout << "  Email     : " << email        << "\n";
    std::cout << "  SĐT       : " << soDienThoai  << "\n";
    std::cout << "  Vai trò   : " << vaiTro       << "\n";
    std::cout << "  Trạng thái: " << (dangOnline ? "🟢 Online" : "🔴 Offline") << "\n";
    std::cout << "========================================\n";
}
