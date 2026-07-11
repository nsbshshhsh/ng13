import http.server
import socketserver
import subprocess
import os
import json
import urllib.parse
import socket
import time
from datetime import datetime, timedelta

# ===========================================================
# Helper: Trích xuất dòng JSON sạch từ stdout của C++
# ===========================================================
def extract_json_from_output(raw_output: bytes) -> bytes:
    """Tìm dòng JSON hợp lệ đầu tiên (từ cuối lên) trong stdout của C++."""
    for enc in ('utf-8', 'cp1258', 'latin-1'):
        try:
            text = raw_output.decode(enc)
            break
        except Exception:
            continue
    else:
        return raw_output

    lines = text.splitlines()
    for line in reversed(lines):
        stripped = line.strip()
        if stripped and (stripped.startswith('{') or stripped.startswith('[')):
            try:
                json.loads(stripped)
                return stripped.encode('utf-8')
            except Exception:
                continue

    return json.dumps({"ok": False, "msg": "C++ did not return valid JSON"}).encode('utf-8')

PORT = 8085
DIRECTORY = "web"

def get_lan_ip():
    """Lay IP LAN de may khac trong cung Wi-Fi/LAN co the truy cap."""
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.connect(("8.8.8.8", 80))
        return sock.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        if sock:
            sock.close()

# BIẾN TOÀN CỤC LƯU TRẠNG THÁI KHÁCH HÀNG ĐÃ BẤM THANH TOÁN (LƯU TRÊN RAM SERVER)
PAID_ORDERS = {}

# BIẾN TOÀN CỤC LƯU TRẠNG THÁI KHÓA GHẾ TẠM THỜI
# Cấu trúc: { "showtimeId": { "seatId": expiration_timestamp_float } }
LOCKED_SEATS = {}

class CinemaHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        # Thêm CORS headers và chống lưu cache trình duyệt
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate")
        self.send_header("Pragma", "no-cache")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200, "ok")
        self.end_headers()

    def do_GET(self):
        if self.path.startswith("/api/"):
            self.handle_api()
        else:
            super().do_GET()

    def do_POST(self):
        if self.path.startswith("/api/"):
            self.handle_api()
        else:
            self.send_error(404, "File not found")

    def handle_api(self):
        parsed_url = urllib.parse.urlparse(self.path)
        # Lấy tên endpoint sạch (ví dụ: "movies", "check-payment")
        endpoint = parsed_url.path.replace("/api/", "")
        query_params = urllib.parse.parse_qs(parsed_url.query)
        
        # Đọc dữ liệu POST body nếu có
        post_data = {}
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length > 0:
            body = self.rfile.read(content_length).decode('utf-8')
            try:
                post_data = json.loads(body)
            except Exception:
                parsed_body = urllib.parse.parse_qs(body)
                post_data = {k: v[0] for k, v in parsed_body.items()}

        # -----------------------------------------------------------
        # XỬ LÝ CÁC API THANH TOÁN TỰ CHẾ (Không chạy qua C++)
        # -----------------------------------------------------------
        if endpoint == "submit-payment":
            # Chấp nhận cả dữ liệu truyền qua POST body hoặc qua GET query cho linh hoạt
            order_id = post_data.get("orderId") or query_params.get("orderId", [None])[0]
            if order_id:
                PAID_ORDERS[str(order_id).strip()] = True  # Đánh dấu đã trả tiền thành công
                
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "msg": "Hệ thống xác thực thành công!"}).encode('utf-8'))
            return
            
        elif endpoint == "check-payment":
            # Đọc mã hóa đơn từ POST body hoặc từ URL (?orderId=...)
            order_id = post_data.get("orderId") or query_params.get("orderId", [None])[0]
            
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            
            # Kiểm tra trạng thái trong dictionary toàn cục
            is_paid = PAID_ORDERS.get(str(order_id).strip(), False) if order_id else False
            
            response_data = {
                "paid": is_paid, 
                "msg": "Thanh toán thành công!" if is_paid else "Đang chờ người dùng quét mã bấm xác nhận..."
            }
            self.wfile.write(json.dumps(response_data).encode('utf-8'))
            return
            
        elif endpoint == "lock-seat":
            st_id = post_data.get("showtimeId")
            seat_id = post_data.get("seatId")
            if st_id and seat_id:
                if st_id not in LOCKED_SEATS:
                    LOCKED_SEATS[st_id] = {}
                # Khóa ghế trong 5 phút (300 giây)
                LOCKED_SEATS[st_id][seat_id] = time.time() + 300
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
            return

        elif endpoint == "unlock-seat":
            st_id = post_data.get("showtimeId")
            seat_id = post_data.get("seatId")
            if st_id and seat_id and st_id in LOCKED_SEATS:
                LOCKED_SEATS[st_id].pop(seat_id, None)
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
            return

        elif endpoint == "locked-seats":
            st_id = query_params.get("showtimeId", [None])[0]
            current_time = time.time()
            locked = []
            if st_id and st_id in LOCKED_SEATS:
                # Xóa các ghế đã hết hạn khóa
                keys_to_delete = []
                for s_id, exp_time in LOCKED_SEATS[st_id].items():
                    if current_time > exp_time:
                        keys_to_delete.append(s_id)
                    else:
                        locked.append(s_id)
                for k in keys_to_delete:
                    del LOCKED_SEATS[st_id][k]
                    
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(locked).encode('utf-8'))
            return

        elif endpoint == "admin-chart":
            # Phân tích bookings.csv để lấy doanh thu 7 ngày qua
            import csv
            chart_data = {"labels": [], "revenue": []}
            revenue_map = {}
            # Sinh 7 ngày gần nhất
            today = datetime.now()
            for i in range(6, -1, -1):
                d = (today - timedelta(days=i)).strftime("%Y-%m-%d")
                revenue_map[d] = 0
            
            try:
                bookings_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "bookings.csv")
                if os.path.exists(bookings_path):
                    with open(bookings_path, "r", encoding="utf-8") as f:
                        for line in f:
                            parts = line.strip().split(",")
                            if len(parts) >= 9:
                                # id(0), user_id(1), date(2), tongTien(3), soTienGiam(4), total(5), payMethod(6), status(7), ticketCode(8)
                                date_str = parts[2].split(" ")[0] # Lấy YYYY-MM-DD
                                total = float(parts[5])
                                status = parts[7]
                                if status == "1" and date_str in revenue_map: # 1 = DA_THANH_TOAN
                                    revenue_map[date_str] += total
            except Exception as e:
                print("Error reading bookings for chart:", e)

            for d in sorted(revenue_map.keys()):
                # Format label (DD/MM)
                lbl = d[8:10] + "/" + d[5:7]
                chart_data["labels"].append(lbl)
                chart_data["revenue"].append(revenue_map[d])

            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(chart_data).encode('utf-8'))
            return

        # -----------------------------------------------------------
        # XỬ LÝ CÁC API KẾT NỐI XUỐNG C++ BACKEND (.EXE)
        # -----------------------------------------------------------
        exe_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "CinemaBooking.exe")
        cmd = [exe_path]
        
        if endpoint == "movies":
            cmd.append("--api-get-movies")
            
        elif endpoint == "showtimes":
            cmd.append("--api-get-showtimes")
            
        elif endpoint == "rooms":
            cmd.append("--api-get-rooms")
            
        elif endpoint == "sold-seats":
            st_id = query_params.get("showtimeId", [""])[0] or post_data.get("showtimeId", "")
            cmd.extend(["--api-get-sold-seats", st_id])
            
        elif endpoint == "login":
            email = post_data.get("email", "")
            password = post_data.get("password", "")
            cmd.extend(["--api-login", email, password])
            
        elif endpoint == "register":
            name = post_data.get("name", "")
            email = post_data.get("email", "")
            phone = post_data.get("phone", "")
            password = post_data.get("password", "")
            cmd.extend(["--api-register", name, email, phone, password])
            
        elif endpoint == "book":
            user_id = str(post_data.get("userId", ""))
            st_id = post_data.get("showtimeId", "")
            
            seats_raw = post_data.get("seats", [])
            if isinstance(seats_raw, list):
                seat_ids = []
                for s in seats_raw:
                    if isinstance(s, dict):
                        seat_ids.append(s.get("id"))
                    else:
                        seat_ids.append(str(s))
                seats = ",".join(seat_ids)
            else:
                seats = str(seats_raw)
                
            pay_method = str(post_data.get("payMethod", "0"))
            points = str(post_data.get("pointsUsed", "0"))
            combo_price = str(post_data.get("comboPrice", "0"))
            cmd.extend(["--api-book", user_id, st_id, seats, pay_method, points, combo_price])
            
        elif endpoint == "tickets":
            uid = query_params.get("userId", [""])[0] or str(post_data.get("userId", ""))
            cmd.extend(["--api-get-tickets", uid])
            
        elif endpoint == "cancel":
            bid = post_data.get("bookingId", "")
            cmd.extend(["--api-cancel-ticket", bid])
            
        elif endpoint == "dashboard":
            cmd.append("--api-get-dashboard")
            
        elif endpoint == "add-movie":
            ten = post_data.get("title", "")
            theloai = post_data.get("genre", "")
            thoiluong = str(post_data.get("duration", "120"))
            mota = post_data.get("desc", "")
            tuoi = str(post_data.get("ageLimit", "0"))
            ngay = post_data.get("releaseDate", "")
            poster = post_data.get("poster", "🎬")
            posterBase64 = post_data.get("posterBase64", None)
            if posterBase64:
                import base64
                try:
                    header, encoded = posterBase64.split(",", 1)
                    ext = "png"
                    if "jpeg" in header or "jpg" in header:
                        ext = "jpg"
                    elif "webp" in header:
                        ext = "webp"
                    filename = f"poster_{int(time.time())}.{ext}"
                    filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "web", "images", filename)
                    with open(filepath, "wb") as f:
                        f.write(base64.b64decode(encoded))
                    poster = f"images/{filename}"
                except Exception as e:
                    print("Error saving poster:", e)
            cmd.extend(["--api-add-movie", ten, theloai, thoiluong, mota, tuoi, ngay, poster])
            
        elif endpoint == "add-showtime":
            maphim = post_data.get("movieId", "")
            maphong = post_data.get("roomId", "")
            thoigian = post_data.get("datetime", "")
            gia = str(post_data.get("price", "80000"))
            ngonngu = post_data.get("lang", "Phụ đề Việt")
            cmd.extend(["--api-add-showtime", maphim, maphong, thoigian, gia, ngonngu])
            
        elif endpoint == "delete-showtime":
            sid = post_data.get("showtimeId", "")
            cmd.extend(["--api-delete-showtime", sid])
            
        elif endpoint == "delete-movie":
            mid = post_data.get("movieId", "")
            cmd.extend(["--api-delete-movie", mid])
            
        elif endpoint == "update-movie":
            mid = post_data.get("movieId", "")
            ten = post_data.get("title", "")
            theloai = post_data.get("genre", "")
            thoiluong = str(post_data.get("duration", "120"))
            mota = post_data.get("desc", "")
            tuoi = str(post_data.get("ageLimit", "0"))
            ngay = post_data.get("releaseDate", "")
            poster = post_data.get("poster", "🎬")
            posterBase64 = post_data.get("posterBase64", None)
            if posterBase64:
                import base64
                try:
                    header, encoded = posterBase64.split(",", 1)
                    ext = "png"
                    if "jpeg" in header or "jpg" in header:
                        ext = "jpg"
                    elif "webp" in header:
                        ext = "webp"
                    filename = f"poster_{int(time.time())}.{ext}"
                    filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "web", "images", filename)
                    with open(filepath, "wb") as f:
                        f.write(base64.b64decode(encoded))
                    poster = f"images/{filename}"
                except Exception as e:
                    print("Error saving poster:", e)
            cmd.extend(["--api-update-movie", mid, ten, theloai, thoiluong, mota, tuoi, ngay, poster])
            
        elif endpoint == "customers":
            cmd.append("--api-get-customers")
        else:
            self.send_error(404, f"Endpoint /api/{endpoint} not supported")
            return

        # Thực thi file C++ CinemaBooking.exe
        try:
            startupinfo = None
            if os.name == 'nt':
                startupinfo = subprocess.STARTUPINFO()
                startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
                startupinfo.wShowWindow = 0

            result = subprocess.run(
                cmd, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE, 
                startupinfo=startupinfo,
                cwd=os.path.dirname(os.path.abspath(__file__))
            )
            
            if result.returncode == 0:
                clean_json = extract_json_from_output(result.stdout)
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(clean_json)
            else:
                self.send_response(500)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                err_msg = result.stderr.decode('utf-8', errors='replace') if result.stderr else f"Exit code {result.returncode}"
                self.wfile.write(json.dumps({"ok": False, "msg": f"C++ Error: {err_msg}"}).encode('utf-8'))
        except Exception as e:
            self.send_response(500)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": False, "msg": f"Gateway Server Error: {str(e)}"}).encode('utf-8'))

class ThreadingHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

if __name__ == "__main__":
    import sys
    sys.stdout.reconfigure(encoding='utf-8')
    # Nhan moi ket noi tu may khac trong cung Wi-Fi/LAN
    lan_ip = get_lan_ip()
    with ThreadingHTTPServer(("0.0.0.0", PORT), CinemaHTTPRequestHandler) as httpd:
        print(f"\n[INFO] Gateway Server running at http://0.0.0.0:{PORT}")
        print(f"[INFO] Local Access: http://localhost:{PORT}")
        print(f"[INFO] LAN Access for other devices: http://{lan_ip}:{PORT}/index.html")
        print(f"[INFO] Serving static files from '{os.path.abspath(DIRECTORY)}' folder")
        print("[INFO] API endpoints mapped to C++ backend 'CinemaBooking.exe'")
        print("Press Ctrl+C to stop.\n")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nStopping server...")