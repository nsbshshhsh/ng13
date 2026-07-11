#include "Booking.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <random>
#include <string>
#include <algorithm>

// ============================================================
// Booking.cpp - Triển khai lớp Đơn đặt vé
// ============================================================

// Bảng ký tự cho mã đặt chỗ (không dùng ký tự dễ nhầm: 0/O, 1/l/I)
static const std::string KY_TU_MA_DAT_CHO = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

// ---- Constructor mặc định ----
Booking::Booking()
    : khachHang(nullptr), ngayDat(""), tongTien(0.0), soTienGiam(0.0),
      soTienThucTe(0.0), tienBapNuoc(0.0), phuongThucThanhToan(PhuongThucThanhToan::TIEN_MAT),
      trangThai(TrangThaiDon::CHO_XAC_NHAN), soTienDaHoan(0.0) {
    maDon = sinhMaDon();
    maDatChoNgauNhien = sinhMaDatChoNgauNhien();
}

// ---- Constructor có tham số ----
Booking::Booking(Customer* khachHang, PhuongThucThanhToan pttt)
    : khachHang(khachHang), tongTien(0.0), soTienGiam(0.0),
      soTienThucTe(0.0), tienBapNuoc(0.0), phuongThucThanhToan(pttt),
      trangThai(TrangThaiDon::CHO_XAC_NHAN), soTienDaHoan(0.0) {
    // Sinh mã đơn và mã đặt chỗ
    maDon = sinhMaDon();
    maDatChoNgauNhien = sinhMaDatChoNgauNhien();

    // Lưu ngày tạo đơn
    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmInfo);
    ngayDat = std::string(buf);
}

Booking::~Booking() {}

// ============================================================
// Sinh mã đặt chỗ ngẫu nhiên: CIN-XXXXXX
// Sử dụng Mersenne Twister RNG để đảm bảo ngẫu nhiên tốt
// ============================================================
std::string Booking::sinhMaDatChoNgauNhien() {
    // Dùng thời gian + random_device làm seed
    std::mt19937 rng(std::random_device{}() ^
                     (uint32_t)time(nullptr));
    std::uniform_int_distribution<int> dist(0, (int)KY_TU_MA_DAT_CHO.size() - 1);

    std::string ma = "CIN-";
    for (int i = 0; i < 6; ++i) {
        ma += KY_TU_MA_DAT_CHO[dist(rng)];
    }
    return ma;  // Ví dụ: CIN-A3F8K2
}

// ============================================================
// Sinh mã đơn hàng: BK-YYYYMMDD-XXXX
// ============================================================
std::string Booking::sinhMaDon() {
    static int dem = 1;  // Bộ đếm tự tăng (trong thực tế lưu vào DB)

    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char ngay[9];
    strftime(ngay, sizeof(ngay), "%Y%m%d", tmInfo);

    std::ostringstream oss;
    oss << "BK-" << ngay << "-"
        << std::setw(4) << std::setfill('0') << dem++;
    return oss.str();
}

// ============================================================
// Tạo đơn từ danh sách vé
// ============================================================
void Booking::taoDonDat(const std::vector<Ticket>& dsVe) {
    danhSachVe = dsVe;
    // Cập nhật mã đơn cho từng vé
    for (auto& ve : danhSachVe) {
        ve.setMaDon(maDon);
    }
    tinhTongTien();
    std::cout << "✅ Tạo đơn đặt vé thành công!\n";
    std::cout << "   Mã đơn  : " << maDon << "\n";
    std::cout << "   Số vé   : " << danhSachVe.size() << " vé\n";
    std::cout << "   Tổng    : " << std::fixed << std::setprecision(0)
              << tongTien << "đ\n";
}

// ============================================================
// Thêm 1 vé vào đơn
// ============================================================
void Booking::themVe(const Ticket& ve) {
    danhSachVe.push_back(ve);
    tinhTongTien();
}

// ============================================================
// Tính tổng tiền đơn hàng
// ============================================================
double Booking::tinhTongTien() {
    tongTien = tienBapNuoc;
    for (const auto& ve : danhSachVe) {
        tongTien += ve.getGiaVe();
    }
    soTienThucTe = tongTien - soTienGiam;
    if (soTienThucTe < 0) soTienThucTe = 0;
    return tongTien;
}

// ============================================================
// Áp dụng giảm giá (điểm tích lũy / voucher)
// ============================================================
void Booking::apDungGiamGia(double soTienGiamMoi) {
    soTienGiam = soTienGiamMoi;
    soTienThucTe = tongTien - soTienGiam;
    if (soTienThucTe < 0) soTienThucTe = 0;
    std::cout << "🎫 Đã áp dụng giảm giá: " << std::fixed << std::setprecision(0)
              << soTienGiam << "đ\n";
    std::cout << "   Số tiền cần trả: " << soTienThucTe << "đ\n";
}

// ============================================================
// Xử lý thanh toán
// ============================================================
bool Booking::thanhToan() {
    if (danhSachVe.empty()) {
        std::cout << "❌ Đơn hàng không có vé nào!\n";
        return false;
    }
    if (trangThai != TrangThaiDon::CHO_XAC_NHAN) {
        std::cout << "❌ Đơn hàng không ở trạng thái chờ thanh toán!\n";
        return false;
    }

    std::cout << "\n💳 Đang xử lý thanh toán "
              << ptttToString(phuongThucThanhToan) << "...\n";
    std::cout << "   Số tiền : " << std::fixed << std::setprecision(0)
              << soTienThucTe << "đ\n";

    // Mô phỏng xử lý thanh toán (trong thực tế gọi Payment Gateway)
    // ...

    // Cập nhật trạng thái đơn hàng
    trangThai = TrangThaiDon::DA_THANH_TOAN;

    // Cập nhật trạng thái từng vé
    for (auto& ve : danhSachVe) {
        ve.setTrangThai(TrangThaiVe::DA_THANH_TOAN);
        // Xác nhận bán ghế
        if (ve.getGhe()) ve.getGhe()->xacNhanBan();
    }

    // Cộng điểm tích lũy cho khách
    if (khachHang) {
        khachHang->congDiemTichLuy(soTienThucTe);
        khachHang->themVaLichSu(maDon);
    }

    std::cout << "✅ Thanh toán thành công!\n";
    hienThiXacNhan();
    return true;
}

// ============================================================
// Tính tiền hoàn dựa trên giờ còn lại trước giờ chiếu
// Chính sách hoàn tiền:
//   > 24h trước chiếu : hoàn 100%
//   1h - 24h trước    : hoàn 50%
//   < 1h trước        : không hoàn (0%)
// ============================================================
double Booking::tinhTienHoan(double gioTruocChieu) const {
    double phanTramHoan = 0.0;

    if (gioTruocChieu > 24.0) {
        phanTramHoan = 1.0;  // Hoàn 100%
        std::cout << "✅ Hủy trước 24h: Hoàn tiền 100%\n";
    } else if (gioTruocChieu >= 1.0) {
        phanTramHoan = 0.5;  // Hoàn 50%
        std::cout << "⚠️  Hủy trong vòng 24h: Hoàn tiền 50%\n";
    } else {
        phanTramHoan = 0.0;  // Không hoàn
        std::cout << "❌ Hủy dưới 1 giờ trước chiếu: Không hoàn tiền\n";
    }

    return soTienThucTe * phanTramHoan;
}

// ============================================================
// Hủy đơn đặt vé
// ============================================================
double Booking::huyDon() {
    if (!coTheHuy()) {
        std::cout << "❌ Đơn hàng không thể hủy!\n";
        return 0.0;
    }

    // Giả lập: tính giờ còn lại trước giờ chiếu
    // Trong thực tế sẽ so sánh ngày giờ thực với giờ chiếu
    double gioTruocChieu = 30.0;  // Mặc định demo: 30h trước chiếu

    // Tính tiền hoàn
    double tienHoan = tinhTienHoan(gioTruocChieu);
    soTienDaHoan = tienHoan;

    // Hủy từng vé và giải phóng ghế
    for (auto& ve : danhSachVe) {
        ve.huyVe();
    }

    // Cập nhật trạng thái đơn
    trangThai = (tienHoan > 0) ? TrangThaiDon::HOAN_TIEN : TrangThaiDon::DA_HUY;

    std::cout << "✅ Đã hủy đơn: " << maDon << "\n";
    std::cout << "💰 Số tiền hoàn: " << std::fixed << std::setprecision(0)
              << tienHoan << "đ\n";
    if (tienHoan > 0) {
        std::cout << "   Tiền sẽ được hoàn trong 3-5 ngày làm việc.\n";
    }

    return tienHoan;
}

// ============================================================
// Kiểm tra có thể hủy không
// ============================================================
bool Booking::coTheHuy() const {
    return trangThai == TrangThaiDon::CHO_XAC_NHAN ||
           trangThai == TrangThaiDon::DA_THANH_TOAN;
}

// ============================================================
// Hiển thị xác nhận đặt vé (in sau khi thanh toán thành công)
// ============================================================
void Booking::hienThiXacNhan() const {
    std::string tenKH = khachHang ? khachHang->getHoTen() : "Khách";

    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════╗\n";
    std::cout << "║         🎉  ĐẶT VÉ THÀNH CÔNG!  🎉                   ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║                                                       ║\n";
    std::cout << "║   🔖 MÃ ĐẶT CHỖ CỦA BẠN:                            ║\n";
    std::cout << "║                                                       ║\n";
    std::cout << "║   ┌─────────────────────┐                            ║\n";
    std::cout << "║   │                     │                            ║\n";
    std::cout << "║   │   " << std::left << std::setw(16) << maDatChoNgauNhien
              << "    │                            ║\n";
    std::cout << "║   │                     │                            ║\n";
    std::cout << "║   └─────────────────────┘                            ║\n";
    std::cout << "║                                                       ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║  Mã đơn   : " << std::left << std::setw(43) << maDon << "║\n";
    std::cout << "║  Khách    : " << std::left << std::setw(43) << tenKH << "║\n";
    std::cout << "║  Số vé    : " << std::left << std::setw(43) << danhSachVe.size() << "║\n";
    std::cout << "║  Tổng tiền: " << std::left << std::setw(40)
              << (std::to_string((int)soTienThucTe) + "đ") << "   ║\n";
    std::cout << "║  Thanh toán: " << std::left << std::setw(42)
              << ptttToString(phuongThucThanhToan) << "║\n";
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║  Danh sách vé:                                        ║\n";
    for (const auto& ve : danhSachVe) {
        std::cout << "║    ";
        ve.inVeNgan();
    }
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║  📱 Vui lòng xuất trình mã đặt chỗ tại quầy rạp     ║\n";
    std::cout << "║  📧 Xác nhận đã gửi đến email của bạn                ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════╝\n";
}

// ============================================================
// Hiển thị chi tiết đơn hàng
// ============================================================
void Booking::hienThiChiTiet() const {
    std::cout << "\n📋 CHI TIẾT ĐƠN ĐẶT VÉ\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "  Mã đơn      : " << maDon << "\n";
    std::cout << "  Mã đặt chỗ  : " << maDatChoNgauNhien << "\n";
    std::cout << "  Ngày đặt    : " << ngayDat << "\n";
    std::cout << "  Khách hàng  : "
              << (khachHang ? khachHang->getHoTen() : "N/A") << "\n";
    std::cout << "  Trạng thái  : " << trangThaiToString(trangThai) << "\n";
    std::cout << "  Thanh toán  : " << ptttToString(phuongThucThanhToan) << "\n";
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "  Danh sách vé (" << danhSachVe.size() << " vé):\n";
    for (const auto& ve : danhSachVe) {
        std::cout << "    ";
        ve.inVeNgan();
    }
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "  Tổng tiền   : " << std::fixed << std::setprecision(0) << tongTien << "đ\n";
    if (tienBapNuoc > 0)
    std::cout << "  Bắp Nước    : " << tienBapNuoc << "đ\n";
    if (soTienGiam > 0)
    std::cout << "  Giảm giá    : -" << soTienGiam << "đ\n";
    std::cout << "  Thực trả    : " << soTienThucTe << "đ\n";
    if (soTienDaHoan > 0)
    std::cout << "  Đã hoàn     : " << soTienDaHoan << "đ\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

// ============================================================
// Chuyển enum sang chuỗi
// ============================================================
std::string Booking::ptttToString(PhuongThucThanhToan pt) {
    switch (pt) {
        case PhuongThucThanhToan::TIEN_MAT:      return "Tiền mặt";
        case PhuongThucThanhToan::THE_NGAN_HANG: return "Thẻ ngân hàng";
        case PhuongThucThanhToan::MOMO:          return "Ví MoMo";
        case PhuongThucThanhToan::VNPAY:         return "VNPay QR";
        case PhuongThucThanhToan::ZALOPAY:       return "ZaloPay";
        case PhuongThucThanhToan::DIEM_TICH_LUY: return "Điểm tích lũy";
        default:                                  return "Không xác định";
    }
}

std::string Booking::trangThaiToString(TrangThaiDon tt) {
    switch (tt) {
        case TrangThaiDon::CHO_XAC_NHAN:  return "⏳ Chờ thanh toán";
        case TrangThaiDon::DA_THANH_TOAN: return "✅ Đã thanh toán";
        case TrangThaiDon::DA_HUY:        return "❌ Đã hủy";
        case TrangThaiDon::HOAN_TIEN:     return "💰 Đang hoàn tiền";
        case TrangThaiDon::DA_HOAN_TIEN:  return "✅ Đã hoàn tiền";
        default:                           return "Không xác định";
    }
}
