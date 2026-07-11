#pragma once
#ifndef SEAT_H
#define SEAT_H

#include <string>
#include <iostream>

// ============================================================
// Lớp Seat - Ghế ngồi trong phòng chiếu
// ============================================================

// Enum loại ghế - ảnh hưởng đến giá vé
enum class LoaiGhe {
    THUONG,   // Ghế thường - giá cơ bản
    VIP,      // Ghế VIP - giá x1.5
    COUPLE,   // Ghế đôi (Sweetbox) - giá x2.0
    DISABLED  // Ghế dành cho người khuyết tật - giá = ghế thường
};

// Enum trạng thái ghế
enum class TrangThaiGhe {
    TRONG,      // Ghế chưa có ai đặt
    DA_DAT,     // Đã được đặt (đang giữ chỗ)
    DA_BAN,     // Đã bán (thanh toán xong)
    KHOA,       // Khóa ghế (hỏng/bảo trì)
    DANG_CHON   // Đang được chọn (trong quá trình đặt)
};

class Seat {
private:
    std::string  maGhe;       // Mã ghế duy nhất (VD: A01, B12)
    char         hang;        // Hàng ghế (A, B, C, ... Z)
    int          soThuTu;     // Số thứ tự trong hàng (1, 2, 3, ...)
    LoaiGhe      loaiGhe;     // Loại ghế (Thường/VIP/Couple)
    TrangThaiGhe trangThai;   // Trạng thái ghế
    double       giaGhe;      // Giá của ghế này (tính từ giá cơ sở + hệ số loại)
    std::string  maDatGhe;    // Mã booking đang giữ ghế này (nếu có)

public:
    // ----- Constructor & Destructor -----
    Seat();
    Seat(const std::string& maGhe, char hang, int soThuTu,
         LoaiGhe loaiGhe, double giaCoSo);
    ~Seat();

    // ----- Getter -----
    std::string  getMaGhe()     const { return maGhe; }
    char         getHang()      const { return hang; }
    int          getSoThuTu()   const { return soThuTu; }
    LoaiGhe      getLoaiGhe()   const { return loaiGhe; }
    TrangThaiGhe getTrangThai() const { return trangThai; }
    double       getGiaGhe()    const { return giaGhe; }
    std::string  getMaDatGhe()  const { return maDatGhe; }

    // ----- Kiểm tra trạng thái -----
    bool laTrong()   const { return trangThai == TrangThaiGhe::TRONG; }
    bool laDaDat()   const { return trangThai == TrangThaiGhe::DA_DAT; }
    bool laDaBan()   const { return trangThai == TrangThaiGhe::DA_BAN; }
    bool laKhoa()    const { return trangThai == TrangThaiGhe::KHOA; }
    bool laDangChon()const { return trangThai == TrangThaiGhe::DANG_CHON; }
    bool coTheDat()  const { return trangThai == TrangThaiGhe::TRONG; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Đặt ghế - chuyển sang trạng thái ĐÃ ĐẶT
     * @param maDon Mã đơn đặt vé
     * @return true nếu đặt thành công (ghế còn trống)
     */
    bool datGhe(const std::string& maDon);

    /**
     * @brief Hủy đặt ghế - trả về trạng thái TRỐNG
     * @return true nếu hủy thành công
     */
    bool huyDatGhe();

    /**
     * @brief Xác nhận bán ghế (sau thanh toán) - chuyển sang ĐÃ BÁN
     * @return true nếu xác nhận thành công
     */
    bool xacNhanBan();

    /**
     * @brief Khóa ghế (bảo trì/hỏng hóc)
     */
    void khoaGhe();

    /**
     * @brief Mở khóa ghế
     */
    void moKhoaGhe();

    /**
     * @brief Tính giá ghế dựa trên loại ghế và giá cơ sở
     * @param giaCoSo Giá vé cơ sở của suất chiếu
     * @return Giá ghế sau khi nhân hệ số
     */
    static double tinhGia(LoaiGhe loai, double giaCoSo);

    /**
     * @brief Hiển thị thông tin ghế
     */
    void hienThiThongTin() const;

    /**
     * @brief Lấy ký hiệu hiển thị trên sơ đồ ghế
     * [_] = trống  [X] = đã đặt  [V] = VIP trống  [C] = Couple
     */
    std::string kyHieuSoDo() const;

    // ----- Hàm tiện ích -----
    static std::string loaiGheToString(LoaiGhe loai);
    static std::string trangThaiToString(TrangThaiGhe tt);
};

#endif // SEAT_H
