#pragma once
#ifndef SHOWTIME_H
#define SHOWTIME_H

#include "Movie.h"
#include "Room.h"
#include <string>
#include <iostream>

// ============================================================
// Lớp Showtime - Suất chiếu phim
// Kết hợp: Phim + Phòng + Thời gian chiếu
// ============================================================

// Enum trạng thái suất chiếu
enum class TrangThaiSuatChieu {
    CHUA_MO_BAN,    // Chưa mở bán vé
    DANG_BAN,       // Đang bán vé (có thể đặt)
    SAP_CHIEU,      // Sắp chiếu (còn < 30 phút, không đặt được)
    DANG_CHIEU,     // Đang chiếu
    DA_CHIEU,       // Đã chiếu xong
    HUY             // Suất bị hủy
};

class Showtime {
private:
    std::string          maSuatChieu;      // Mã suất chiếu (VD: ST20241115001)
    Movie*               phim;             // Pointer đến phim
    Room*                phongChieu;       // Pointer đến phòng chiếu
    std::string          thoiGianBatDau;   // Giờ bắt đầu (YYYY-MM-DD HH:MM)
    std::string          thoiGianKetThuc;  // Giờ kết thúc (tự tính từ thời lượng phim)
    std::string          ngayChieu;        // Ngày chiếu (YYYY-MM-DD)
    double               giaVeCoSo;        // Giá vé cơ sở của suất này
    TrangThaiSuatChieu   trangThai;        // Trạng thái suất chiếu
    bool                 laSuatDacBiet;    // Suất đặc biệt (tăng giá 20%)
    std::string          ngonNguPhim;      // Thuyết minh / Phụ đề
    std::string          maCinema;         // Rạp chiếu nào

public:
    // ----- Constructor & Destructor -----
    Showtime();
    Showtime(const std::string& maSuatChieu,
             Movie* phim,
             Room*  phongChieu,
             const std::string& thoiGianBatDau,
             double giaVeCoSo,
             const std::string& ngonNguPhim = "Phụ đề Việt");
    ~Showtime();

    // ----- Getter -----
    std::string        getMaSuatChieu()    const { return maSuatChieu; }
    Movie*             getPhim()           const { return phim; }
    Room*              getPhongChieu()     const { return phongChieu; }
    std::string        getThoiGianBatDau() const { return thoiGianBatDau; }
    std::string        getThoiGianKetThuc()const { return thoiGianKetThuc; }
    std::string        getNgayChieu()      const { return ngayChieu; }
    double             getGiaVeCoSo()      const { return giaVeCoSo; }
    TrangThaiSuatChieu getTrangThai()      const { return trangThai; }
    bool               isLaSuatDacBiet()  const { return laSuatDacBiet; }
    std::string        getNgonNguPhim()   const { return ngonNguPhim; }
    std::string        getMaCinema()      const { return maCinema; }

    // ----- Setter -----
    void setTrangThai(TrangThaiSuatChieu tt)     { trangThai = tt; }
    void setLaSuatDacBiet(bool dacBiet)          { laSuatDacBiet = dacBiet; }
    void setGiaVeCoSo(double gia)                { giaVeCoSo = gia; }
    void setMaCinema(const std::string& ma)      { maCinema = ma; }
    void setMaSuatChieu(const std::string& ma)   { maSuatChieu = ma; }
    void setPhim(Movie* p)                       { phim = p; }
    void setPhongChieu(Room* r)                  { phongChieu = r; }
    void setThoiGianBatDau(const std::string& t) { thoiGianBatDau = t; }
    void setThoiGianKetThuc(const std::string& t){ thoiGianKetThuc = t; }
    void setNgayChieu(const std::string& n)      { ngayChieu = n; }
    void setNgonNguPhim(const std::string& n)    { ngonNguPhim = n; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Hiển thị thông tin chi tiết suất chiếu
     * Gồm: tên phim, phòng, giờ chiếu, số ghế trống, giá vé
     */
    void hienThiChiTiet() const;

    /**
     * @brief Hiển thị thông tin tóm tắt (dùng trong danh sách)
     */
    void hienThiTomTat() const;

    /**
     * @brief Đếm số ghế còn trống trong suất chiếu này
     * @return Số ghế trống
     */
    int kiemTraGheTrong() const;

    /**
     * @brief Kiểm tra suất chiếu có còn bán vé không
     * @return true nếu đang bán và còn ghế
     */
    bool coTheDatVe() const;

    /**
     * @brief Tính giá vé cụ thể cho loại ghế trong suất này
     * Áp dụng: giá cơ sở + hệ số ghế + phụ phí suất đặc biệt
     * @param loaiGhe Loại ghế
     * @return Giá vé (đồng)
     */
    double tinhGiaVe(LoaiGhe loaiGhe) const;

    /**
     * @brief Tính giờ kết thúc dựa trên giờ bắt đầu + thời lượng phim
     * @return Chuỗi giờ kết thúc (HH:MM)
     */
    std::string tinhGioKetThuc() const;

    /**
     * @brief Kiểm tra có phải suất chiếu cuối ngày không (sau 22h)
     */
    bool laSuatCuoiNgay() const;

    // ----- Hàm tiện ích -----
    static std::string trangThaiToString(TrangThaiSuatChieu tt);
};

#endif // SHOWTIME_H
