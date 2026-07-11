#pragma once
#ifndef TICKET_H
#define TICKET_H

#include "Showtime.h"
#include "Seat.h"
#include <string>
#include <iostream>

// ============================================================
// Lớp Ticket - Vé xem phim
// Mỗi vé tương ứng với 1 ghế trong 1 suất chiếu
// ============================================================

// Enum trạng thái vé
enum class TrangThaiVe {
    CHO_THANH_TOAN,  // Vé vừa được tạo, chờ thanh toán
    DA_THANH_TOAN,   // Đã thanh toán xong, vé hợp lệ
    DA_SU_DUNG,      // Đã quét QR tại rạp (đã vào xem)
    DA_HUY,          // Đã hủy (trước giờ chiếu)
    HET_HAN          // Hết hạn (quá giờ chiếu, chưa dùng)
};

class Ticket {
private:
    std::string   maVe;           // Mã vé duy nhất (VD: TK-20241115-0001)
    Showtime*     suatChieu;      // Suất chiếu của vé này
    Seat*         ghe;            // Ghế ngồi được đặt
    double        giaVe;          // Giá vé thực tế (sau khi tính)
    TrangThaiVe   trangThai;      // Trạng thái của vé
    std::string   maQR;           // Mã QR để quét tại rạp
    std::string   ngayTao;        // Ngày tạo vé (YYYY-MM-DD HH:MM:SS)
    std::string   tenKhachHang;   // Tên khách hàng (lưu lại để in vé)
    std::string   emailKhachHang; // Email khách hàng (gửi vé điện tử)
    std::string   maDon;          // Mã đơn đặt vé chứa vé này

public:
    // ----- Constructor & Destructor -----
    Ticket();
    Ticket(const std::string& maVe,
           Showtime* suatChieu,
           Seat* ghe,
           const std::string& tenKhachHang,
           const std::string& emailKhachHang,
           const std::string& maDon = "");
    ~Ticket();

    // ----- Getter -----
    std::string  getMaVe()           const { return maVe; }
    Showtime*    getSuatChieu()      const { return suatChieu; }
    Seat*        getGhe()            const { return ghe; }
    double       getGiaVe()          const { return giaVe; }
    TrangThaiVe  getTrangThai()      const { return trangThai; }
    std::string  getMaQR()           const { return maQR; }
    std::string  getNgayTao()        const { return ngayTao; }
    std::string  getTenKhachHang()   const { return tenKhachHang; }
    std::string  getMaDon()          const { return maDon; }

    // ----- Setter -----
    void setTrangThai(TrangThaiVe tt)       { trangThai = tt; }
    void setMaDon(const std::string& ma)    { maDon = ma; }
    void setMaVe(const std::string& ma)             { maVe = ma; }
    void setSuatChieu(Showtime* s)                  { suatChieu = s; }
    void setGhe(Seat* g)                            { ghe = g; }
    void setGiaVe(double gia)                       { giaVe = gia; }
    void setMaQR(const std::string& qr)             { maQR = qr; }
    void setNgayTao(const std::string& ngay)        { ngayTao = ngay; }
    void setTenKhachHang(const std::string& ten)    { tenKhachHang = ten; }
    void setEmailKhachHang(const std::string& email){ emailKhachHang = email; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Tính giá vé dựa trên loại ghế và suất chiếu
     * Công thức: giaCoSo × heSoGhe × (1.2 nếu suất đặc biệt)
     * @return Giá vé (đồng)
     */
    double tinhGiaVe();

    /**
     * @brief Tạo mã QR chứa thông tin vé
     * Định dạng: BASE64(maVe|maSuatChieu|maGhe|ngayTao)
     * @return Chuỗi mã QR (ASCII art simulation)
     */
    std::string taoMaQR();

    /**
     * @brief In thông tin vé ra màn hình (mô phỏng in vé vật lý)
     * Bao gồm: tên phim, giờ chiếu, ghế, giá, mã QR
     */
    void inVe() const;

    /**
     * @brief In vé dạng ngắn gọn (cho email xác nhận)
     */
    void inVeNgan() const;

    /**
     * @brief Đánh dấu vé đã được sử dụng (quét QR tại rạp)
     * @return true nếu thành công (vé hợp lệ và chưa dùng)
     */
    bool suDungVe();

    /**
     * @brief Hủy vé (chỉ hủy được khi chưa quá giờ chiếu)
     * @return true nếu hủy thành công
     */
    bool huyVe();

    /**
     * @brief Kiểm tra vé có hợp lệ để vào xem không
     */
    bool laHopLe() const;

    // ----- Hàm tiện ích -----
    static std::string trangThaiToString(TrangThaiVe tt);
    static std::string taoMaVeMoi(int soThuTu);
};

#endif // TICKET_H
