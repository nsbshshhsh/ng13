#include "DatabaseManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

std::string DatabaseManager::folderPath = "data/";

void DatabaseManager::setFolderPath(const std::string& path) {
    folderPath = path;
}

std::string DatabaseManager::getFolderPath() {
    return folderPath;
}

bool DatabaseManager::fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat((folderPath + filename).c_str(), &buffer) == 0);
}

void DatabaseManager::ensureDirectoryExists() {
#ifdef _WIN32
    _mkdir(folderPath.c_str());
#else
    mkdir(folderPath.c_str(), 0777);
#endif
}

int DatabaseManager::safeStoi(const std::string& s, int def) {
    if (s.empty()) return def;
    try {
        return std::stoi(s);
    } catch (...) {
        return def;
    }
}

double DatabaseManager::safeStod(const std::string& s, double def) {
    if (s.empty()) return def;
    try {
        // Handle comma as decimal separator in different locales
        std::string s_copy = s;
        for (char& c : s_copy) {
            if (c == ',') c = '.';
        }
        return std::stod(s_copy);
    } catch (...) {
        return def;
    }
}

std::vector<std::string> DatabaseManager::parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::string cell;
    bool inQuotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(cell);
            cell.clear();
        } else {
            cell += c;
        }
    }
    result.push_back(cell);
    return result;
}

std::string DatabaseManager::escapeCSV(const std::string& field) {
    bool needQuotes = false;
    if (field.find(',') != std::string::npos || field.find('"') != std::string::npos || field.find('\n') != std::string::npos) {
        needQuotes = true;
    }
    if (!needQuotes) return field;
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') escaped += "\"\"";
        else escaped += c;
    }
    escaped += "\"";
    return escaped;
}

// ============================================================
// LƯU PHIM
// ============================================================
void DatabaseManager::saveMovies(const std::vector<Movie>& movies) {
    ensureDirectoryExists();
    std::ofstream file(folderPath + "movies.csv");
    if (!file.is_open()) return;

    for (const auto& m : movies) {
        file << escapeCSV(m.getMaPhim()) << ","
             << escapeCSV(m.getTenPhim()) << ","
             << escapeCSV(m.getTenPhimGoc()) << ","
             << escapeCSV(m.getTheLoai()) << ","
             << m.getThoiLuong() << ","
             << escapeCSV(m.getNgayKhoiChieu()) << ","
             << escapeCSV(m.getNgayKetThucChieu()) << ","
             << escapeCSV(m.getMoTa()) << ","
             << m.getGioiHanTuoi() << ","
             << escapeCSV(m.getDaoDien()) << ","
             << escapeCSV(m.getDienVienChinh()) << ","
             << escapeCSV(m.getPoster()) << ","
             << m.getDiemDanhGia() << ","
             << m.getSoLuotDanhGia() << ","
             << (m.isDangChieu() ? 1 : 0) << ","
             << escapeCSV(m.getTrailerUrl()) << "\n";
    }
    file.close();
}

// ============================================================
// ĐỌC PHIM
// ============================================================
std::vector<Movie> DatabaseManager::loadMovies() {
    std::vector<Movie> movies;
    std::ifstream file(folderPath + "movies.csv");
    if (!file.is_open()) return movies;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cells = parseCSVLine(line);
        if (cells.size() < 16) continue;

        Movie m(cells[0], cells[1], cells[3], safeStoi(cells[4]), cells[5], cells[7], safeStoi(cells[8]));
        m.setTenPhimGoc(cells[2]);
        m.setNgayKetThucChieu(cells[6]);
        m.setDaoDien(cells[9]);
        m.setDienVienChinh(cells[10]);
        m.setPoster(cells[11]);
        m.setDiemDanhGia(safeStod(cells[12]));
        m.setSoLuotDanhGia(safeStoi(cells[13]));
        m.setDangChieu(safeStoi(cells[14]) != 0);
        m.setTrailerUrl(cells[15]);

        movies.push_back(m);
    }
    file.close();
    return movies;
}

// ============================================================
// LƯU PHÒNG
// ============================================================
void DatabaseManager::saveRooms(const std::vector<Room>& rooms) {
    ensureDirectoryExists();
    std::ofstream file(folderPath + "rooms.csv");
    if (!file.is_open()) return;

    for (const auto& r : rooms) {
        file << escapeCSV(r.getMaPhong()) << ","
             << escapeCSV(r.getTenPhong()) << ","
             << static_cast<int>(r.getLoaiPhong()) << ","
             << r.getSoHang() << ","
             << r.getSoGheMoiHang() << ","
             << r.getTongSoGhe() << ","
             << (r.isDangHoatDong() ? 1 : 0) << ","
             << escapeCSV(r.getMaCinema()) << "\n";
    }
    file.close();
}

// ============================================================
// ĐỌC PHÒNG
// ============================================================
std::vector<Room> DatabaseManager::loadRooms(const std::string& maCinema) {
    std::vector<Room> rooms;
    std::ifstream file(folderPath + "rooms.csv");
    if (!file.is_open()) return rooms;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cells = parseCSVLine(line);
        if (cells.size() < 8) continue;

        Room r(cells[0], cells[1], static_cast<LoaiPhong>(safeStoi(cells[2])), 
               safeStoi(cells[3]), safeStoi(cells[4]), cells[7]);
        r.setDangHoatDong(safeStoi(cells[6]) != 0);
        r.setMaCinema(cells[7]);
        
        // Khởi tạo sơ đồ ghế trống cơ bản
        r.khoiTaoGhe(); 

        rooms.push_back(r);
    }
    file.close();
    return rooms;
}

// ============================================================
// LƯU TÀI KHOẢN
// ============================================================

void DatabaseManager::saveUsers(const std::vector<Customer>& customers, const std::vector<Admin>& admins) {
    ensureDirectoryExists();
    std::ofstream file(folderPath + "users.csv");
    if (!file.is_open()) return;

    // Ép dòng stream luôn ghi số thực ở dạng thông thường, không dùng số mũ khoa học
    file << std::fixed << std::setprecision(0);

    // Định dạng: id,role,hoTen,email,soDienThoai,matKhau,diemTichLuy,tongTienDaMua,hangThanhVien,quyenHan,phongBan,maNhanVien
    for (const auto& c : customers) {
        file << c.getId() << ",customer,"
             << escapeCSV(c.getHoTen()) << ","
             << escapeCSV(c.getEmail()) << ","
             << escapeCSV(c.getSoDienThoai()) << ","
             << escapeCSV(c.getMatKhau()) << ","
             << c.getDiemTichLuy() << ","
             << c.getTongTienDaMua() << "," // Nhờ có lệnh trên, số này sẽ ra 4728000 tròn trịa
             << escapeCSV(c.getHangThanhVien()) << ",0,,,\n";
    }
    for (const auto& a : admins) {
        file << a.getId() << ",admin,"
             << escapeCSV(a.getHoTen()) << ","
             << escapeCSV(a.getEmail()) << ","
             << escapeCSV(a.getSoDienThoai()) << ","
             << escapeCSV(a.getMatKhau()) << ","
             << "0,0,,," 
             << static_cast<int>(a.getQuyenHan()) << ","
             << escapeCSV(a.getPhongBan()) << ","
             << escapeCSV(a.getMaNhanVien()) << "\n";
    }
    file.close();
}

// ============================================================
// ĐỌC TÀI KHOẢN
// ============================================================
void DatabaseManager::loadUsers(std::vector<Customer>& customers, std::vector<Admin>& admins) {
    customers.clear();
    admins.clear();
    std::ifstream file(folderPath + "users.csv");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cells = parseCSVLine(line);
        if (cells.size() < 6) continue;

        int id = safeStoi(cells[0]);
        std::string role = cells[1];
        std::string name = cells[2];
        std::string email = cells[3];
        std::string sdt = cells[4];
        std::string hashedPw = cells[5];

        if (role == "customer") {
            Customer c(id, name, email, sdt, "");
            c.setMatKhauHashed(hashedPw);
            if (cells.size() >= 9) {
                c.setDiemTichLuy(safeStoi(cells[6]));
                c.setTongTienDaMua(safeStod(cells[7]));
                c.setHangThanhVien(cells[8]);
            }
            customers.push_back(c);
        } else if (role == "admin") {
            Admin a(id, name, email, sdt, "", QuyenHan::STAFF, "Vận hành");
            a.setMatKhauHashed(hashedPw);
            a.setVaiTro("admin");
            if (cells.size() >= 13) {
                a.setQuyenHan(static_cast<QuyenHan>(safeStoi(cells[10])));
                a.setPhongBan(cells[11]);
                a.setMaNhanVien(cells[12]);
            }
            admins.push_back(a);
        }
    }
    file.close();
}

// ============================================================
// LƯU SUẤT CHIẾU
// ============================================================
void DatabaseManager::saveShowtimes(const std::vector<Showtime>& showtimes) {
    ensureDirectoryExists();
    std::ofstream file(folderPath + "showtimes.csv");
    if (!file.is_open()) return;

    for (const auto& s : showtimes) {
        file << escapeCSV(s.getMaSuatChieu()) << ","
             << escapeCSV(s.getPhim() ? s.getPhim()->getMaPhim() : "") << ","
             << escapeCSV(s.getPhongChieu() ? s.getPhongChieu()->getMaPhong() : "") << ","
             << escapeCSV(s.getThoiGianBatDau()) << ","
             << escapeCSV(s.getThoiGianKetThuc()) << ","
             << escapeCSV(s.getNgayChieu()) << ","
             << s.getGiaVeCoSo() << ","
             << static_cast<int>(s.getTrangThai()) << ","
             << (s.isLaSuatDacBiet() ? 1 : 0) << ","
             << escapeCSV(s.getNgonNguPhim()) << ","
             << escapeCSV(s.getMaCinema()) << "\n";
    }
    file.close();
}

// ============================================================
// ĐỌC SUẤT CHIẾU
// ============================================================
std::vector<Showtime> DatabaseManager::loadShowtimes(std::vector<Movie>& movies, std::vector<Room>& rooms) {
    std::vector<Showtime> showtimes;
    std::ifstream file(folderPath + "showtimes.csv");
    if (!file.is_open()) return showtimes;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cells = parseCSVLine(line);
        if (cells.size() < 11) continue;

        std::string maSuat = cells[0];
        std::string maPhim = cells[1];
        std::string maPhong = cells[2];
        std::string tgBD = cells[3];
        std::string tgKT = cells[4];
        std::string ngay = cells[5];
        double gia = safeStod(cells[6]);
        TrangThaiSuatChieu tt = static_cast<TrangThaiSuatChieu>(safeStoi(cells[7]));
        bool dacBiet = safeStoi(cells[8]) != 0;
        std::string ngonNgu = cells[9];
        std::string maCine = cells[10];

        // Tìm con trỏ phim và phòng
        Movie* targetMovie = nullptr;
        for (auto& m : movies) {
            if (m.getMaPhim() == maPhim) {
                targetMovie = &m;
                break;
            }
        }

        Room* targetRoom = nullptr;
        for (auto& r : rooms) {
            if (r.getMaPhong() == maPhong) {
                targetRoom = &r;
                break;
            }
        }

        Showtime s(maSuat, targetMovie, targetRoom, tgBD, gia, ngonNgu);
        s.setThoiGianKetThuc(tgKT);
        s.setNgayChieu(ngay);
        s.setTrangThai(tt);
        s.setLaSuatDacBiet(dacBiet);
        s.setMaCinema(maCine);

        showtimes.push_back(s);
    }
    file.close();
    return showtimes;
}

// ============================================================
// LƯU ĐƠN HÀNG VÀ VÉ
// ============================================================
void DatabaseManager::saveBookingsAndTickets(const std::vector<Booking>& bookings) {
    ensureDirectoryExists();
    std::ofstream fileB(folderPath + "bookings.csv");
    std::ofstream fileT(folderPath + "tickets.csv");
    if (!fileB.is_open() || !fileT.is_open()) return;

    for (const auto& b : bookings) {
        fileB << escapeCSV(b.getMaDon()) << ","
              << (b.getKhachHang() ? b.getKhachHang()->getId() : 0) << ","
              << escapeCSV(b.getNgayDat()) << ","
              << b.getTongTien() << ","
              << b.getSoTienGiam() << ","
              << b.getSoTienThucTe() << ","
              << static_cast<int>(b.getPhuongThucThanhToan()) << ","
              << static_cast<int>(b.getTrangThai()) << ","
              << escapeCSV(b.getMaDatChoNgauNhien()) << ","
              << b.getSoTienDaHoan() << "\n";

        for (const auto& t : b.getDanhSachVe()) {
            fileT << escapeCSV(t.getMaVe()) << ","
                  << escapeCSV(b.getMaDon()) << ","
                  << escapeCSV(t.getSuatChieu() ? t.getSuatChieu()->getMaSuatChieu() : "") << ","
                  << escapeCSV(t.getGhe() ? t.getGhe()->getMaGhe() : "") << ","
                  << escapeCSV(t.getTenKhachHang()) << ","
                  << escapeCSV(t.getMaQR()) << ","
                  << t.getGiaVe() << ","
                  << static_cast<int>(t.getTrangThai()) << ","
                  << escapeCSV(t.getNgayTao()) << "\n";
        }
    }
    fileB.close();
    fileT.close();
}

// ============================================================
// ĐỌC ĐƠN HÀNG VÀ VÉ
// ============================================================
std::vector<Booking> DatabaseManager::loadBookingsAndTickets(std::vector<Customer>& customers, const std::vector<Showtime>& showtimes) {
    std::vector<Booking> bookings;
    std::ifstream fileB(folderPath + "bookings.csv");
    if (!fileB.is_open()) return bookings;

    // Load tất cả vé trước
    struct TempTicket {
        std::string maVe;
        std::string maDon;
        std::string maSuatChieu;
        std::string maGhe;
        std::string tenKH;
        std::string maQR;
        double giaVe;
        TrangThaiVe trangThai;
        std::string ngayTao;
    };
    std::vector<TempTicket> tempTickets;
    std::ifstream fileT(folderPath + "tickets.csv");
    if (fileT.is_open()) {
        std::string line;
        while (std::getline(fileT, line)) {
            if (line.empty()) continue;
            std::vector<std::string> cells = parseCSVLine(line);
            if (cells.size() < 9) continue;
            tempTickets.push_back({
                cells[0], cells[1], cells[2], cells[3], cells[4], cells[5],
                safeStod(cells[6]), static_cast<TrangThaiVe>(safeStoi(cells[7])), cells[8]
            });
        }
        fileT.close();
    }

    std::string line;
    while (std::getline(fileB, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cells = parseCSVLine(line);
        if (cells.size() < 10) continue;

        std::string maDon = cells[0];
        int userId = safeStoi(cells[1]);
        std::string ngayDat = cells[2];
        double tongTien = safeStod(cells[3]);
        double giam = safeStod(cells[4]);
        double thucTe = safeStod(cells[5]);
        PhuongThucThanhToan pttt = static_cast<PhuongThucThanhToan>(safeStoi(cells[6]));
        TrangThaiDon tt = static_cast<TrangThaiDon>(safeStoi(cells[7]));
        std::string maGheDat = cells[8];
        double hoan = safeStod(cells[9]);

        Customer* targetCustomer = nullptr;
        for (auto& c : customers) {
            if (c.getId() == userId) {
                targetCustomer = &c;
                break;
            }
        }

        Booking b(targetCustomer, pttt);
        b.setMaDon(maDon);
        b.setNgayDat(ngayDat);
        b.setTongTien(tongTien);
        b.setSoTienGiam(giam);
        b.setSoTienThucTe(thucTe);
        b.setTrangThai(tt);
        b.setMaDatChoNgauNhien(maGheDat);
        b.setSoTienDaHoan(hoan);

        // Nạp vé thuộc đơn hàng này
        std::vector<Ticket> bVe;
        for (const auto& t : tempTickets) {
            if (t.maDon == maDon) {
                Showtime* targetSuat = nullptr;
                for (const auto& s : showtimes) {
                    if (s.getMaSuatChieu() == t.maSuatChieu) {
                        targetSuat = const_cast<Showtime*>(&s);
                        break;
                    }
                }

                Seat* targetSeat = nullptr;
                if (targetSuat && targetSuat->getPhongChieu()) {
                    targetSeat = targetSuat->getPhongChieu()->timGheBangMa(t.maGhe);
                }

                Ticket realTicket(t.maVe, targetSuat, targetSeat, t.tenKH, 
                                  targetCustomer ? targetCustomer->getEmail() : "", maDon);
                realTicket.setTrangThai(t.trangThai);
                realTicket.setMaQR(t.maQR);
                realTicket.setNgayTao(t.ngayTao);
                realTicket.setGiaVe(t.giaVe);

                bVe.push_back(realTicket);

                // Nếu vé hợp lệ (chưa bị hủy), mark ghế là đã đặt
                if (t.trangThai == TrangThaiVe::DA_THANH_TOAN || t.trangThai == TrangThaiVe::DA_SU_DUNG) {
                    if (targetSeat) {
                        targetSeat->datGhe(maDon);
                    }
                }
            }
        }
        b.setDanhSachVe(bVe);
        bookings.push_back(b);

        // Cập nhật lịch sử đặt vé cho khách hàng
        if (targetCustomer && (tt == TrangThaiDon::DA_THANH_TOAN || tt == TrangThaiDon::DA_HUY || tt == TrangThaiDon::DA_HOAN_TIEN)) {
            targetCustomer->themVaLichSu(maDon);
        }
    }
    fileB.close();
    return bookings;
}

// ============================================================
// ĐỒNG BỘ NẠP TOÀN BỘ
// ============================================================
void DatabaseManager::loadAllData(std::vector<Movie>& movies, 
                                  CinemaRoom& rap, 
                                  std::vector<Customer>& customers, 
                                  std::vector<Admin>& admins, 
                                  std::vector<Booking>& bookings, 
                                  MovieSchedule& schedule) {
    std::cout << "📥 Đang tải dữ liệu từ CSV...\n";
    movies = loadMovies();
    
    // Tải phòng
    std::vector<Room> rooms = loadRooms(rap.getMaCinema());
    rap.getDanhSachPhong().clear();
    for (auto& r : rooms) {
        rap.getDanhSachPhong().push_back(r);
    }

    // Load users
    loadUsers(customers, admins);

    // Tải lịch chiếu
    // Đồng bộ danh sách phim và phòng vào schedule trước
    schedule.getDanhSachPhimRef().clear();
    for (auto& m : movies) {
        schedule.themPhimVaoHeThong(&m);
    }
    schedule.getDanhSachPhongRef().clear();
    for (auto& r : rap.getDanhSachPhong()) {
        schedule.themPhongVaoHeThong(&r);
    }

    std::vector<Showtime> showtimes = loadShowtimes(movies, rap.getDanhSachPhong());
    schedule.getDanhSachSuatChieuRef().clear();
    for (auto& s : showtimes) {
        schedule.getDanhSachSuatChieuRef().push_back(s);
    }

    // Load đơn đặt vé & vé
    bookings = loadBookingsAndTickets(customers, schedule.getDanhSachSuatChieuRef());
    std::cout << "✅ Tải dữ liệu thành công!\n";
}

// ============================================================
// ĐỒNG BỘ GHI TOÀN BỘ
// ============================================================
void DatabaseManager::saveAllData(const std::vector<Movie>& movies, 
                                  const CinemaRoom& rap, 
                                  const std::vector<Customer>& customers, 
                                  const std::vector<Admin>& admins, 
                                  const std::vector<Booking>& bookings, 
                                  const MovieSchedule& schedule) {
    saveMovies(movies);
    saveRooms(rap.getDanhSachPhong());
    saveUsers(customers, admins);
    saveShowtimes(schedule.getDanhSachSuatChieu());
    saveBookingsAndTickets(bookings);
}

// ============================================================
// TẠO DỮ LIỆU MẪU BAN ĐẦU
// ============================================================
void DatabaseManager::seedDefaultData(std::vector<Movie>& movies, 
                                      CinemaRoom& rap, 
                                      std::vector<Customer>& customers, 
                                      std::vector<Admin>& admins, 
                                      MovieSchedule& schedule) {
    if (fileExists("movies.csv") && fileExists("users.csv") && fileExists("rooms.csv")) {
        // Đã có dữ liệu, nạp lên
        std::vector<Booking> bookings;
        loadAllData(movies, rap, customers, admins, bookings, schedule);
        return;
    }

    std::cout << "🌱 Khởi tạo dữ liệu mẫu lần đầu ghi ra CSV...\n";
    // === TẠO RẠP CHIẾU PHIM ===
    rap = CinemaRoom("CGV_HN01", "CGV Vincom Bà Triệu",
                     "191 Bà Triệu, Hai Bà Trưng", "Hà Nội", "CGV");
    rap.setSoDienThoai("1900 6017");
    rap.setGioMoCua("08:00");
    rap.setGioDongCua("23:30");

    rap.themPhong("P01", "Phòng 1", LoaiPhong::HAI_D,    8, 12, 80000.0);
    rap.themPhong("P02", "Phòng 2", LoaiPhong::BA_D,     8, 12, 100000.0);
    rap.themPhong("P03", "Phòng 3", LoaiPhong::IMAX,     9, 15, 150000.0);
    rap.themPhong("P04", "Phòng 4", LoaiPhong::PREMIUM,  6, 10, 200000.0);

    for (auto& p : rap.getDanhSachPhong()) {
        schedule.themPhongVaoHeThong(&p);
    }

    // === TẠO PHIM MẪU ===
    Movie phim1("MV001", "Avengers: Secret Wars", "Hành động/Siêu anh hùng",
                180, "2026-05-01", "Trận chiến cuối cùng của các Avengers chống lại Doctor Doom.", 13);
    phim1.setTenPhimGoc("Avengers: Secret Wars");
    phim1.setDaoDien("Russo Brothers");
    phim1.setDienVienChinh("Robert Downey Jr., Chris Evans");
    phim1.setTrailerUrl("https://youtube.com/watch?v=example1");
    phim1.themDanhGia(9.2);
    phim1.themDanhGia(8.8);
    phim1.setPoster("🦸");

    Movie phim2("MV002", "Gia Đình Là Số 1",  "Tình cảm/Hài hước",
                120, "2026-04-20", "Câu chuyện về gia đình và tình yêu thương.", 0);
    phim2.setTenPhimGoc("Family Is First");
    phim2.setDaoDien("Victor Vu");
    phim2.setDienVienChinh("Trấn Thành, Hari Won");
    phim2.themDanhGia(8.5);
    phim2.setPoster("👨‍👩‍👧‍👦");

    Movie phim3("MV003", "Vong Bóng Tối", "Kinh dị",
                105, "2026-05-10", "Một gia đình chuyển đến ngôi nhà ma ám...", 18);
    phim3.setTenPhimGoc("Shadow of Darkness");
    phim3.setDaoDien("James Wan");
    phim3.setDienVienChinh("Patrick Wilson");
    phim3.themDanhGia(7.8);
    phim3.setPoster("👻");

    Movie phim4("MV004", "Doraemon: Nobita Ở Đây", "Hoạt hình",
                95, "2026-05-05", "Doraemon và Nobita trong cuộc phiêu lưu mới.", 0);
    phim4.setTenPhimGoc("Doraemon: New Adventure");
    phim4.setDaoDien("Yoshihiro Shimizu");
    phim4.setPoster("🤖");

    movies.push_back(phim1);
    movies.push_back(phim2);
    movies.push_back(phim3);
    movies.push_back(phim4);

    for (auto& p : movies) {
        schedule.themPhimVaoHeThong(&p);
    }

    // === TẠO LỊCH CHIẾU MẪU ===
    schedule.themBophimVaoLich("MV001", "P03", "2026-06-10 09:30", 150000.0, "Phụ đề Việt");
    schedule.themBophimVaoLich("MV001", "P02", "2026-06-10 14:00", 100000.0, "Thuyết minh");
    schedule.themBophimVaoLich("MV001", "P03", "2026-06-10 19:30", 150000.0, "Phụ đề Việt", true);
    schedule.themBophimVaoLich("MV002", "P01", "2026-06-10 10:00", 80000.0,  "Thuyết minh");
    schedule.themBophimVaoLich("MV002", "P01", "2026-06-10 15:30", 80000.0,  "Thuyết minh");
    schedule.themBophimVaoLich("MV003", "P02", "2026-06-10 21:00", 100000.0, "Phụ đề Việt");
    schedule.themBophimVaoLich("MV004", "P01", "2026-06-10 13:00", 80000.0,  "Thuyết minh");

    // === TẠO TÀI KHOẢN MẪU ===
    admins.emplace_back(1, "Nguyễn Quản Trị", "admin@cinema.vn", "0901234567", "admin123", QuyenHan::SUPER_ADMIN, "Quản lý hệ thống");
    customers.emplace_back(101, "Trần Văn An", "an.tran@gmail.com", "0912345678", "123456");
    customers.emplace_back(102, "Lê Thị Bình", "binh.le@gmail.com", "0923456789", "123456");
    customers[0].congDiemTichLuy(500000); // 50 điểm

    // Ghi ra CSV lần đầu
    saveAllData(movies, rap, customers, admins, {}, schedule);
}
