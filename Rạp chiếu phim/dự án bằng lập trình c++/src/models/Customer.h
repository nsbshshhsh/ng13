#pragma once
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "User.h"
#include <vector>
#include <string>

// ============================================================
// Lớp Customer - Khách hàng (Kế thừa từ User)
// Đây là loại tài khoản dùng để đặt vé, xem lịch chiếu
// ============================================================
class Customer : public User {
private:
    int                      diemTichLuy;    // Điểm tích lũy (1000đ = 1 điểm)
    std::vector<std::string> lichSuDatVe;   // Danh sách mã đơn đặt vé đã đặt
    std::string              hangThanhVien;  // BRONZE / SILVER / GOLD / DIAMOND
    double                   tongTienDaMua;  // Tổng số tiền đã chi tiêu (để xếp hạng)

public:
    // ----- Constructor & Destructor -----
    Customer();
    Customer(int id, const std::string& hoTen, const std::string& email,
             const std::string& soDienThoai, const std::string& matKhau);
    virtual ~Customer();

    // ----- Getter -----
    int                      getDiemTichLuy()   const { return diemTichLuy; }
    std::vector<std::string> getLichSuDatVe()   const { return lichSuDatVe; }
    std::string              getHangThanhVien()  const { return hangThanhVien; }
    double                   getTongTienDaMua()  const { return tongTienDaMua; }

    // ----- Setter -----
    void setDiemTichLuy(int diem) { diemTichLuy = diem; }
    void setTongTienDaMua(double tong) { tongTienDaMua = tong; }
    void setHangThanhVien(const std::string& hang) { hangThanhVien = hang; }
    void setLichSuDatVe(const std::vector<std::string>& ls) { lichSuDatVe = ls; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Xem danh sách phim và lịch chiếu hiện có
     * Hiển thị bảng phim đang chiếu, giờ chiếu, phòng
     */
    void xemLichChieu() const;

    /**
     * @brief Thực hiện quy trình đặt vé
     * Bước 1: Chọn phim → Bước 2: Chọn suất → Bước 3: Chọn ghế → Thanh toán
     * @param maSuatChieu Mã suất chiếu muốn đặt
     * @param danhSachMaGhe Danh sách mã ghế muốn chọn
     * @return Mã đơn đặt vé nếu thành công, chuỗi rỗng nếu thất bại
     */
    std::string datVe(const std::string& maSuatChieu,
                      const std::vector<std::string>& danhSachMaGhe);

    /**
     * @brief Hủy vé đã đặt và xử lý hoàn tiền
     * - Hủy trước 24h: hoàn 100%
     * - Hủy trong 1-24h: hoàn 50%
     * - Hủy dưới 1h: không hoàn tiền
     * @param maDon Mã đơn đặt vé cần hủy
     * @return true nếu hủy thành công
     */
    bool huyVe(const std::string& maDon);

    /**
     * @brief Xem lịch sử tất cả vé đã đặt
     */
    void xemLichSu() const;

    /**
     * @brief Cộng điểm tích lũy sau khi mua vé thành công
     * Cứ 10.000đ = 1 điểm
     * @param soTien Số tiền đã thanh toán
     */
    void congDiemTichLuy(double soTien);

    /**
     * @brief Đổi điểm tích lũy để được giảm giá
     * 100 điểm = giảm 50.000đ
     * @param soDiem Số điểm muốn đổi
     * @return Số tiền được giảm (đồng)
     */
    double doiDiem(int soDiem);

    /**
     * @brief Cập nhật hạng thành viên dựa trên tổng chi tiêu
     * BRONZE: < 1 triệu | SILVER: 1-5 triệu | GOLD: 5-20 triệu | DIAMOND: > 20 triệu
     */
    void capNhatHangThanhVien();

    /**
     * @brief Thêm mã đơn đặt vé vào lịch sử
     */
    void themVaLichSu(const std::string& maDon);

    /**
     * @brief Hiển thị thông tin khách hàng (override)
     */
    void hienThiThongTin() const override;

    // Override phương thức thuần ảo
    std::string loaiNguoiDung() const override { return "Khách hàng"; }
};

#endif // CUSTOMER_H
