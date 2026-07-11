#include "Customer.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// ============================================================
// Customer.cpp - Triển khai lớp Khách hàng
// ============================================================

// ---- Constructor mặc định ----
Customer::Customer()
    : User(), diemTichLuy(0), hangThanhVien("BRONZE"), tongTienDaMua(0.0) {
    vaiTro = "customer";
}

// ---- Constructor có tham số ----
Customer::Customer(int id, const std::string& hoTen, const std::string& email,
                   const std::string& soDienThoai, const std::string& matKhau)
    : User(id, hoTen, email, soDienThoai, matKhau, "customer"),
      diemTichLuy(0), hangThanhVien("BRONZE"), tongTienDaMua(0.0) {}

// ---- Destructor ----
Customer::~Customer() {}

// ============================================================
// Xem lịch chiếu (thông báo sẽ được kết nối với MovieService)
// ============================================================
void Customer::xemLichChieu() const {
    std::cout << "\n🎬 LỊCH CHIẾU PHIM\n";
    std::cout << "══════════════════════════════════════════════\n";
    std::cout << "  [Dữ liệu lịch chiếu sẽ được tải từ hệ thống]\n";
    std::cout << "  Vui lòng sử dụng MovieService.layLichChieu()\n";
    std::cout << "══════════════════════════════════════════════\n";
}

// ============================================================
// Đặt vé (logic chính trong BookingService)
// ============================================================
std::string Customer::datVe(const std::string& maSuatChieu,
                             const std::vector<std::string>& danhSachMaGhe) {
    if (!dangOnline) {
        std::cout << "❌ Bạn cần đăng nhập trước khi đặt vé!\n";
        return "";
    }
    if (danhSachMaGhe.empty()) {
        std::cout << "❌ Vui lòng chọn ít nhất 1 ghế!\n";
        return "";
    }

    std::cout << "🎟️  Đang xử lý đặt vé cho suất chiếu: " << maSuatChieu << "\n";
    std::cout << "   Ghế đã chọn: ";
    for (const auto& ghe : danhSachMaGhe) {
        std::cout << ghe << " ";
    }
    std::cout << "\n";
    std::cout << "   [Kết nối BookingService để hoàn tất]\n";

    // Trả về mã đơn mẫu (trong thực tế BookingService sẽ tạo mã thực)
    return "BOOKING_PENDING";
}

// ============================================================
// Hủy vé và hoàn tiền
// ============================================================
bool Customer::huyVe(const std::string& maDon) {
    if (!dangOnline) {
        std::cout << "❌ Bạn cần đăng nhập trước!\n";
        return false;
    }

    // Kiểm tra mã đơn có trong lịch sử không
    bool timThay = false;
    for (const auto& don : lichSuDatVe) {
        if (don == maDon) {
            timThay = true;
            break;
        }
    }

    if (!timThay) {
        std::cout << "❌ Không tìm thấy đơn đặt vé: " << maDon << "\n";
        return false;
    }

    std::cout << "🔄 Đang xử lý hủy vé: " << maDon << "\n";
    std::cout << "   [Kết nối PaymentService để xử lý hoàn tiền]\n";
    return true;
}

// ============================================================
// Xem lịch sử đặt vé
// ============================================================
void Customer::xemLichSu() const {
    std::cout << "\n📋 LỊCH SỬ ĐẶT VÉ\n";
    std::cout << "══════════════════════════════════════════════\n";

    if (lichSuDatVe.empty()) {
        std::cout << "  (Chưa có vé nào được đặt)\n";
    } else {
        std::cout << std::left
                  << std::setw(5)  << "STT"
                  << std::setw(20) << "Mã Đơn"
                  << "\n";
        std::cout << "----------------------------------------------\n";
        for (size_t i = 0; i < lichSuDatVe.size(); ++i) {
            std::cout << std::setw(5)  << (i + 1)
                      << std::setw(20) << lichSuDatVe[i]
                      << "\n";
        }
    }
    std::cout << "══════════════════════════════════════════════\n";
}

// ============================================================
// Cộng điểm tích lũy: cứ 10.000đ = 1 điểm
// ============================================================
void Customer::congDiemTichLuy(double soTien) {
    int diemMoi = static_cast<int>(soTien / 10000.0);
    diemTichLuy += diemMoi;
    tongTienDaMua += soTien;
    
    std::cout << "⭐ Cộng " << diemMoi << " điểm tích lũy! Tổng: "
              << diemTichLuy << " điểm\n";
    
    capNhatHangThanhVien();
}

// ============================================================
// Đổi điểm: 100 điểm = giảm 50.000đ
// ============================================================
double Customer::doiDiem(int soDiem) {
    if (soDiem <= 0 || soDiem > diemTichLuy) {
        std::cout << "❌ Số điểm không hợp lệ! Bạn có " 
                  << diemTichLuy << " điểm.\n";
        return 0.0;
    }
    if (soDiem % 100 != 0) {
        std::cout << "❌ Số điểm phải là bội số của 100!\n";
        return 0.0;
    }

    double tienGiam = (soDiem / 100.0) * 50000.0;
    diemTichLuy -= soDiem;
    std::cout << "✅ Đổi " << soDiem << " điểm → Giảm "
              << std::fixed << std::setprecision(0) << tienGiam << "đ\n";
    return tienGiam;
}

// ============================================================
// Cập nhật hạng thành viên dựa trên tổng chi tiêu
// ============================================================
void Customer::capNhatHangThanhVien() {
    std::string hangCu = hangThanhVien;
    
    if (tongTienDaMua >= 20000000) {
        hangThanhVien = "DIAMOND 💎";
    } else if (tongTienDaMua >= 5000000) {
        hangThanhVien = "GOLD 🥇";
    } else if (tongTienDaMua >= 1000000) {
        hangThanhVien = "SILVER 🥈";
    } else {
        hangThanhVien = "BRONZE 🥉";
    }

    // Thông báo nếu lên hạng
    if (hangThanhVien != hangCu) {
        std::cout << "🎉 Chúc mừng! Bạn đã lên hạng " << hangThanhVien << "!\n";
    }
}

// ============================================================
// Thêm mã đơn vào lịch sử
// ============================================================
void Customer::themVaLichSu(const std::string& maDon) {
    lichSuDatVe.push_back(maDon);
}

// ============================================================
// Hiển thị thông tin khách hàng (Override)
// ============================================================
void Customer::hienThiThongTin() const {
    User::hienThiThongTin();  // Gọi phương thức của lớp cha
    std::cout << "  Hạng TV    : " << hangThanhVien << "\n";
    std::cout << "  Điểm TL    : " << diemTichLuy << " điểm\n";
    std::cout << "  Tổng chi   : " << std::fixed << std::setprecision(0)
              << tongTienDaMua << "đ\n";
    std::cout << "  Số đơn     : " << lichSuDatVe.size() << " đơn\n";
    std::cout << "========================================\n";
}
