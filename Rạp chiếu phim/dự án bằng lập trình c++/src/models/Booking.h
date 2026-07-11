#pragma once
#ifndef BOOKING_H
#define BOOKING_H

#include "Ticket.h"
#include "Customer.h"
#include <string>
#include <vector>
#include <iostream>

// ============================================================
// Lớp Booking - Đơn đặt vé
// Tập hợp nhiều vé (Ticket) thành 1 đơn hàng hoàn chỉnh
// ============================================================

// Enum phương thức thanh toán
enum class PhuongThucThanhToan {
    TIEN_MAT,      // Tiền mặt tại quầy
    THE_NGAN_HANG, // Thẻ tín dụng / ghi nợ
    MOMO,          // Ví MoMo
    VNPAY,         // VNPay QR
    ZALOPAY,       // ZaloPay
    DIEM_TICH_LUY  // Đổi điểm tích lũy
};

// Enum trạng thái đơn hàng
enum class TrangThaiDon {
    CHO_XAC_NHAN,   // Mới tạo, chờ xác nhận thanh toán
    DA_THANH_TOAN,  // Đã thanh toán xong
    DA_HUY,         // Đã hủy
    HOAN_TIEN,      // Đang xử lý hoàn tiền
    DA_HOAN_TIEN    // Hoàn tiền xong
};

class Booking {
private:
    std::string              maDon;               // Mã đơn hàng (VD: BK-A3F8K2)
    Customer*                khachHang;           // Khách hàng đặt vé
    std::vector<Ticket>      danhSachVe;          // Danh sách các vé trong đơn
    std::string              ngayDat;             // Ngày giờ tạo đơn
    double                   tongTien;            // Tổng tiền (chưa giảm)
    double                   soTienGiam;          // Số tiền được giảm
    double                   soTienThucTe;        // Số tiền thực tế phải trả
    double                   tienBapNuoc;         // Tiền bắp nước (Combo Food & Beverage)
    PhuongThucThanhToan      phuongThucThanhToan; // Phương thức thanh toán
    TrangThaiDon             trangThai;           // Trạng thái đơn hàng
    std::string              maDatChoNgauNhien;   // Mã đặt chỗ 8 ký tự (VD: CIN-A3F8K2)
    std::string              ghiChu;              // Ghi chú thêm
    double                   soTienDaHoan;        // Số tiền đã được hoàn

public:
    // ----- Constructor & Destructor -----
    Booking();
    Booking(Customer* khachHang, PhuongThucThanhToan pttt = PhuongThucThanhToan::THE_NGAN_HANG);
    ~Booking();

    // ----- Getter -----
    std::string          getMaDon()               const { return maDon; }
    Customer*            getKhachHang()           const { return khachHang; }
    std::vector<Ticket>& getDanhSachVe()                { return danhSachVe; }
    const std::vector<Ticket>& getDanhSachVe()    const { return danhSachVe; }
    std::string          getNgayDat()             const { return ngayDat; }
    double               getTongTien()            const { return tongTien; }
    double               getSoTienGiam()          const { return soTienGiam; }
    double               getSoTienThucTe()        const { return soTienThucTe; }
    double               getTienBapNuoc()         const { return tienBapNuoc; }
    PhuongThucThanhToan  getPhuongThucThanhToan() const { return phuongThucThanhToan; }
    TrangThaiDon         getTrangThai()           const { return trangThai; }
    std::string          getMaDatChoNgauNhien()   const { return maDatChoNgauNhien; }
    int                  getSoVe()                const { return (int)danhSachVe.size(); }
    double               getSoTienDaHoan()        const { return soTienDaHoan; }

    // ----- Setter -----
    void setGhiChu(const std::string& gc)              { ghiChu = gc; }
    void setPhuongThucThanhToan(PhuongThucThanhToan pt) { phuongThucThanhToan = pt; }
    void setMaDon(const std::string& md)                { maDon = md; }
    void setKhachHang(Customer* kh)                     { khachHang = kh; }
    void setDanhSachVe(const std::vector<Ticket>& ds)   { danhSachVe = ds; }
    void setNgayDat(const std::string& nd)              { ngayDat = nd; }
    void setTongTien(double tt)                         { tongTien = tt; }
    void setSoTienGiam(double sg)                       { soTienGiam = sg; }
    void setSoTienThucTe(double st)                     { soTienThucTe = st; }
    void setTienBapNuoc(double bn)                      { tienBapNuoc = bn; }
    void setTrangThai(TrangThaiDon tt)                  { trangThai = tt; }
    void setMaDatChoNgauNhien(const std::string& ma)    { maDatChoNgauNhien = ma; }
    void setSoTienDaHoan(double th)                     { soTienDaHoan = th; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Tạo đơn đặt vé từ danh sách vé
     * Sinh mã đơn + mã đặt chỗ ngẫu nhiên, gom vé vào đơn
     * @param danhSachVe Danh sách vé cần thêm vào đơn
     */
    void taoDonDat(const std::vector<Ticket>& dsVe);

    /**
     * @brief Thêm 1 vé vào đơn hiện tại
     * @param ve Vé cần thêm
     */
    void themVe(const Ticket& ve);

    /**
     * @brief Tính tổng tiền của đơn (tổng giá tất cả vé)
     * @return Tổng tiền (đồng)
     */
    double tinhTongTien();

    /**
     * @brief Áp dụng mã giảm giá hoặc điểm tích lũy
     * @param soTienGiam Số tiền được giảm
     */
    void apDungGiamGia(double soTienGiam);

    /**
     * @brief Xử lý thanh toán - chuyển trạng thái sang DA_THANH_TOAN
     * Cập nhật trạng thái ghế và vé, sinh mã đặt chỗ QR
     * @return true nếu thanh toán thành công
     */
    bool thanhToan();

    /**
     * @brief Hủy toàn bộ đơn đặt vé
     * Giải phóng ghế, cập nhật trạng thái, tính hoàn tiền
     * @return Số tiền được hoàn (đồng)
     */
    double huyDon();

    /**
     * @brief Tính số tiền hoàn lại khi hủy vé
     * - Hủy trước 24h: hoàn 100%
     * - Hủy trong 1-24h: hoàn 50%
     * - Hủy dưới 1h: hoàn 0% (không hoàn)
     * @param gioTruocChieu Số giờ còn lại trước giờ chiếu
     * @return Số tiền được hoàn (đồng)
     */
    double tinhTienHoan(double gioTruocChieu) const;

    /**
     * @brief Hiển thị xác nhận đơn đặt vé (sau khi thanh toán)
     * In đẹp mã đặt chỗ, danh sách vé, tổng tiền
     */
    void hienThiXacNhan() const;

    /**
     * @brief In toàn bộ thông tin đơn hàng
     */
    void hienThiChiTiet() const;

    /**
     * @brief Kiểm tra đơn có thể hủy không
     */
    bool coTheHuy() const;

    // ----- Hàm tiện ích static -----

    /**
     * @brief Sinh mã đặt chỗ ngẫu nhiên 8 ký tự
     * Định dạng: CIN-XXXXXX (X = chữ hoa hoặc số)
     * @return Chuỗi mã đặt chỗ (VD: "CIN-A3F8K2")
     */
    static std::string sinhMaDatChoNgauNhien();

    /**
     * @brief Sinh mã đơn hàng tự động
     * Định dạng: BK-YYYYMMDD-XXXX
     */
    static std::string sinhMaDon();

    static std::string ptttToString(PhuongThucThanhToan pt);
    static std::string trangThaiToString(TrangThaiDon tt);
};

#endif // BOOKING_H
