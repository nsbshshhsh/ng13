#pragma once
#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <vector>
#include <iostream>

// ============================================================
// Lớp Movie - Thông tin bộ phim
// ============================================================

// Enum thể loại phim
enum class TheLoaiPhim {
    HANH_DONG,   // Hành động
    TINH_CAM,    // Tình cảm / Lãng mạn
    KINH_DI,     // Kinh dị
    HAI_HUOC,    // Hài hước
    HOAT_HINH,   // Hoạt hình
    TAI_LIEU,    // Tài liệu
    KHOA_HOC,    // Khoa học viễn tưởng
    VO_THUAT,    // Võ thuật
    LICH_SU,     // Lịch sử / Cổ trang
    GIA_DINH     // Gia đình
};

class Movie {
private:
    std::string maPhim;          // Mã phim duy nhất (VD: MV2024001)
    std::string tenPhim;         // Tên phim tiếng Việt
    std::string tenPhimGoc;      // Tên phim gốc (tiếng Anh/ngôn ngữ gốc)
    std::string theLoai;         // Thể loại phim (text)
    int         thoiLuong;       // Thời lượng phim (phút)
    std::string ngayKhoiChieu;   // Ngày bắt đầu chiếu (YYYY-MM-DD)
    std::string ngayKetThucChieu;// Ngày kết thúc chiếu (YYYY-MM-DD)
    std::string moTa;            // Mô tả / Synopsis
    int         gioiHanTuoi;     // Giới hạn tuổi: 0, 13, 16, 18
    std::string daoDien;         // Tên đạo diễn
    std::string dienVienChinh;   // Diễn viên chính
    std::string ngonNgu;         // Ngôn ngữ gốc
    std::string poster;          // Đường dẫn ảnh poster
    double      diemDanhGia;     // Điểm đánh giá (0.0 - 10.0)
    int         soLuotDanhGia;   // Số lượt đánh giá
    bool        dangChieu;       // Trạng thái đang chiếu hay không
    std::string trailerUrl;      // Link trailer YouTube

public:
    // ----- Constructor & Destructor -----
    Movie();
    Movie(const std::string& maPhim, const std::string& tenPhim,
          const std::string& theLoai, int thoiLuong,
          const std::string& ngayKhoiChieu, const std::string& moTa,
          int gioiHanTuoi = 0);
    ~Movie();

    // ----- Getter -----
    std::string getMaPhim()           const { return maPhim; }
    std::string getTenPhim()          const { return tenPhim; }
    std::string getTenPhimGoc()       const { return tenPhimGoc; }
    std::string getTheLoai()          const { return theLoai; }
    int         getThoiLuong()        const { return thoiLuong; }
    std::string getNgayKhoiChieu()    const { return ngayKhoiChieu; }
    std::string getNgayKetThucChieu() const { return ngayKetThucChieu; }
    std::string getMoTa()             const { return moTa; }
    int         getGioiHanTuoi()      const { return gioiHanTuoi; }
    std::string getDaoDien()          const { return daoDien; }
    std::string getDienVienChinh()    const { return dienVienChinh; }
    std::string getPoster()           const { return poster; }
    double      getDiemDanhGia()      const { return diemDanhGia; }
    int         getSoLuotDanhGia()    const { return soLuotDanhGia; }
    bool        isDangChieu()         const { return dangChieu; }
    std::string getTrailerUrl()       const { return trailerUrl; }

    // ----- Setter -----
    void setTenPhim(const std::string& ten)     { tenPhim = ten; }
    void setTenPhimGoc(const std::string& ten)  { tenPhimGoc = ten; }
    void setMoTa(const std::string& mota)       { moTa = mota; }
    void setDaoDien(const std::string& dao)     { daoDien = dao; }
    void setDienVienChinh(const std::string& dv){ dienVienChinh = dv; }
    void setPoster(const std::string& path)     { poster = path; }
    void setTrailerUrl(const std::string& url)  { trailerUrl = url; }
    void setNgayKetThucChieu(const std::string& ngay) { ngayKetThucChieu = ngay; }
    void setDangChieu(bool trangThai)           { dangChieu = trangThai; }
    void setMaPhim(const std::string& ma)       { maPhim = ma; }
    void setTheLoai(const std::string& tl)      { theLoai = tl; }
    void setThoiLuong(int tl)                   { thoiLuong = tl; }
    void setNgayKhoiChieu(const std::string& n) { ngayKhoiChieu = n; }
    void setGioiHanTuoi(int tuoi)               { gioiHanTuoi = tuoi; }
    void setDiemDanhGia(double diem)            { diemDanhGia = diem; }
    void setSoLuotDanhGia(int so)               { soLuotDanhGia = so; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Hiển thị toàn bộ thông tin bộ phim
     */
    void hienThiThongTin() const;

    /**
     * @brief Hiển thị thông tin tóm tắt (dùng trong danh sách)
     */
    void hienThiTomTat() const;

    /**
     * @brief Kiểm tra xem phim còn đang trong thời gian công chiếu không
     * @return true nếu phim đang chiếu (ngày hiện tại trong khoảng chiếu)
     */
    bool kiemTraDangChieu() const;

    /**
     * @brief Thêm đánh giá mới cho phim
     * @param diem Điểm đánh giá (1.0 - 10.0)
     */
    void themDanhGia(double diem);

    /**
     * @brief Chuyển thời lượng phút sang chuỗi "X giờ Y phút"
     * @return Chuỗi thời lượng (VD: "2 giờ 15 phút")
     */
    std::string formatThoiLuong() const;

    /**
     * @brief Lấy nhãn giới hạn tuổi (VD: "T13", "T16", "T18", "P")
     */
    std::string getNhanGioiHanTuoi() const;

    /**
     * @brief Kiểm tra phim có phù hợp với độ tuổi không
     * @param tuoiKhachHang Tuổi khách hàng
     */
    bool kiemTraPhuHopTuoi(int tuoiKhachHang) const;
};

#endif // MOVIE_H
