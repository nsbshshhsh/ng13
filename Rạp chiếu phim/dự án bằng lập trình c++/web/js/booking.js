// ══════════════════════════════════════════════════════════
// booking.js – Booking page logic connected to C++ Backend
// ══════════════════════════════════════════════════════════
const bs = {
  // booking state
  movie: null,
  showtime: null,
  selectedSeats: [], // [{id,type,price}]
  comboItems: { 1: 0, 2: 0 },
  comboTotal: 0,
  payMethod: "vnpay",
  pointsUsed: 0,
  subtotal: 0,
  memberDisc: 0,
  pointsDisc: 0,
  total: 0,
};

// ── INIT ──
document.addEventListener("DOMContentLoaded", async () => {
  await window.dbInitialized; // Chờ nạp dữ liệu từ backend C++

  // Retry nếu cache rỗng (server chưa khởi động kịp)
  let retries = 0;
  while (RoomService.getAll().length === 0 && retries < 3) {
    retries++;
    console.warn(`Cache rỗng, thử lại lần ${retries}/3...`);
    await new Promise((r) => setTimeout(r, 1500 * retries));
    await initAppDB();
  }

  if (RoomService.getAll().length === 0) {
    console.error(
      "Không thể kết nối server sau 3 lần thử. Kiểm tra server.py có đang chạy không.",
    );
  }

  const p = new URLSearchParams(window.location.search);
  const movieId = p.get("movie");
  const showtimeId = p.get("showtime");

  if (showtimeId) {
    const suat = ShowtimeService.getById(showtimeId);
    if (suat) {
      bs.showtime = suat;
      // Tải động danh sách ghế đã bán từ C++ API trước khi render
      try {
        const res = await fetch(
          resolveRootPath() + `api/sold-seats?showtimeId=${showtimeId}`,
        );
        const soldSeats = await res.json();
        ShowtimeService.setSoldSeats(showtimeId, soldSeats);
      } catch (err) {
        console.error("Lỗi nạp ghế đã bán:", err);
      }
      loadMovieByShowtime(suat);
    }
  } else if (movieId) {
    const m = MovieService.getById(movieId);
    if (m) loadMovieInfo(m);
    renderShowtimes(movieId);
  } else {
    window.location.href = "movies.html";
  }
  showMemberInfo();
});

function loadMovieByShowtime(suat) {
  const m = MovieService.getById(suat.movieId);
  if (m) {
    loadMovieInfo(m);
    highlightShowtime(suat.id);
  }
  renderShowtimes(suat.movieId, suat.id);
  renderSeatMap(suat);
  updateSummaryHeader(suat, m);
  setStep(2);
}

function loadMovieInfo(m) {
  bs.movie = m;
  document.title = `Đặt vé – ${m.title} | CineMax`;
  const posterEl = document.getElementById("moviePosterEl");
  if (posterEl) posterEl.innerHTML = getPosterHTML(m.poster);
  const titleEl = document.getElementById("movieTitleEl");
  if (titleEl) titleEl.textContent = m.title;
  const metaEl = document.getElementById("movieMetaEl");
  if (metaEl) {
    metaEl.innerHTML = `
      <span class="chip">⏱ ${fmtDuration(m.duration)}</span>
      <span class="chip">${m.genre}</span>
      <span class="chip" style="color:${m.ageLimit === 0 ? "#1db954" : "#e50914"}">${m.ageLimit === 0 ? "P" : "T" + m.ageLimit}</span>
      <span class="chip">⭐ ${m.rating}/10</span>`;
  }
}

// ── SHOWTIMES ──
function renderShowtimes(movieId, activeId) {
  const suats = ShowtimeService.getByMovie(movieId);
  const grid = document.getElementById("showtimeGrid");
  if (!grid) return;
  if (!suats.length) {
    grid.innerHTML =
      '<p style="color:var(--text-400);font-size:.85rem">Không có suất chiếu nào.</p>';
    return;
  }

  // Group by date
  const byDate = {};
  suats.forEach((s) => {
    (byDate[s.date] = byDate[s.date] || []).push(s);
  });

  grid.innerHTML = Object.entries(byDate)
    .map(([date, arr]) => {
      const dayLabel = fmtDate(date);
      const opts = arr
        .map((s) => {
          const room = RoomService.getById(s.roomId);
          const avail =
            s.availableSeats !== undefined
              ? s.availableSeats
              : ShowtimeService.countAvailable(s.id);
          const cls = avail === 0 ? "full" : avail < 15 ? "low" : "";
          const avTxt =
            avail === 0
              ? "❌ Hết vé"
              : avail < 15
                ? `⚠️ Còn ${avail} ghế`
                : `✅ Còn ${avail} ghế`;
          const onclickFn =
            avail === 0
              ? `showToast('Suất này đã hết vé!','error')`
              : `selectShowtime('${s.id}')`;
          return `<div class="showtime-opt ${cls} ${s.id === activeId ? "active" : ""}" id="sopt_${s.id}"
          onclick="${onclickFn}">
        <div class="so-time">${s.time}${s.special ? " ⭐" : ""}</div>
        <div class="so-room">${room?.name || s.roomId} · ${room?.type || ""}</div>
        <div class="so-room">${s.lang}</div>
        <div class="so-avail">${avTxt}</div>
      </div>`;
        })
        .join("");
      return `<div style="grid-column:1/-1;font-size:.78rem;font-weight:700;color:var(--text-400);margin-top:4px">${dayLabel}</div>${opts}`;
    })
    .join("");
}

function highlightShowtime(id) {
  document
    .querySelectorAll(".showtime-opt")
    .forEach((el) => el.classList.remove("active"));
  document.getElementById("sopt_" + id)?.classList.add("active");
}

async function selectShowtime(id) {
  const suat = ShowtimeService.getById(id);
  if (!suat) {
    showToast(
      "Không tìm thấy suất chiếu này. Vui lòng tải lại trang.",
      "error",
    );
    return;
  }

  const movie = MovieService.getById(suat.movieId);
  bs.showtime = suat;
  bs.selectedSeats = [];
  highlightShowtime(id);
  if (movie && movie !== bs.movie) loadMovieInfo(movie);

  // Tải ghế đã bán từ C++ API
  try {
    const res = await fetch(
      resolveRootPath() + `api/sold-seats?showtimeId=${id}`,
    );
    const soldSeats = await res.json();
    ShowtimeService.setSoldSeats(id, soldSeats);
  } catch (err) {
    console.error("Lỗi nạp ghế đã bán:", err);
  }

  // Render sơ đồ ghế (sau khi có sold-seats)
  renderSeatMap(suat);
  updateSummaryHeader(suat, movie);
  setStep(2);

  // Bắt đầu cập nhật ghế khóa tạm thời liên tục
  if (window.seatPollInterval) clearInterval(window.seatPollInterval);
  refreshLockedSeats(id);
  window.seatPollInterval = setInterval(() => refreshLockedSeats(id), 5000);

  // Đợi DOM render xong rồi mới scroll — tránh scroll đến element chưa có kích thước
  requestAnimationFrame(() => {
    setTimeout(() => {
      const section = document.getElementById("seatmapSection");
      if (section) {
        section.scrollIntoView({ behavior: "smooth", block: "nearest" });
        // Flash animation để user nhận ra sơ đồ ghế vừa xuất hiện
        section.style.outline = "2px solid var(--primary)";
        section.style.transition = "outline 0.4s ease";
        setTimeout(() => {
          section.style.outline = "none";
        }, 800);
      }
    }, 100);
  });
}

// ── SEAT MAP ──
const VIP_ROWS = ["D", "E", "F"];
const COUPLE_ROWS = ["H", "I"];

function renderSeatMap(suat) {
  // Ẩn placeholder, hiện nội dung thật
  const placeholder = document.getElementById("seatmapPlaceholder");
  if (placeholder) placeholder.style.display = "none";
  const content = document.getElementById("seatmapContent");
  if (content) content.style.display = "block";
  let room = RoomService.getById(suat.roomId);
  if (!room) {
    // Dự phòng: tạo room object tạm từ thông tin showtime
    console.warn(
      "Room không tìm thấy trong cache, dùng dữ liệu dự phòng cho roomId:",
      suat.roomId,
    );
    room = {
      id: suat.roomId,
      name: suat.roomId,
      type: suat.roomId.includes("IMAX") ? "IMAX" : "2D",
      rows: 9,
      seatsPerRow: 13,
      basePrice: 100000,
    };
  }
  const titleEl = document.getElementById("seatmapTitle");
  if (titleEl) titleEl.textContent = `Phòng: ${room.name}`;
  const subEl = document.getElementById("seatmapSub");
  if (subEl)
    subEl.textContent = `${room.type} · ${suat.lang} · ${suat.time} ngày ${suat.date}`;

  const sold = ShowtimeService.getSoldSeats(suat.id);
  const grid = document.getElementById("seatGrid");
  if (!grid) return;
  const rows = room.rows || 9;
  const cols = room.seatsPerRow || 13;
  grid.innerHTML = "";

  for (let r = 0; r < rows; r++) {
    const letter = String.fromCharCode(65 + r);
    const isVIP = VIP_ROWS.includes(letter);
    const isCouple = COUPLE_ROWS.includes(letter);

    const rowEl = document.createElement("div");
    rowEl.className = "seat-row";

    const lbl = document.createElement("div");
    lbl.className = "row-lbl";
    lbl.textContent = letter;
    rowEl.appendChild(lbl);

    const wrap = document.createElement("div");
    wrap.style.cssText =
      "display:flex;gap:6px;flex-wrap:wrap;align-items:center";

    if (isCouple) {
      for (let s = 1; s <= cols; s += 2) {
        const sid = `${letter}${String(s).padStart(2, "0")}`;
        const isSold = sold.includes(sid);
        const el = makeSeat(sid, "couple", isSold, suat, `♥ ${s}–${s + 1}`);
        wrap.appendChild(el);
        if (s === Math.floor(cols / 2) - 1) {
          const a = document.createElement("div");
          a.className = "aisle";
          wrap.appendChild(a);
        }
      }
    } else {
      for (let s = 1; s <= cols; s++) {
        const sid = `${letter}${String(s).padStart(2, "0")}`;
        const isSold = sold.includes(sid);
        const el = makeSeat(
          sid,
          isVIP ? "vip" : "normal",
          isSold,
          suat,
          String(s),
        );
        wrap.appendChild(el);
        if (s === Math.ceil(cols / 2)) {
          const a = document.createElement("div");
          a.className = "aisle";
          wrap.appendChild(a);
        }
      }
    }
    rowEl.appendChild(wrap);
    grid.appendChild(rowEl);
  }

  const btnGoToCombo = document.getElementById("btnGoToCombo");
  if (btnGoToCombo) btnGoToCombo.style.display = bs.selectedSeats.length > 0 ? "block" : "none";
}

function makeSeat(id, type, isSold, suat, label) {
  const el = document.createElement("div");
  const price = ShowtimeService.calcSeatPrice(suat.id, type);
  el.className = `seat ${type} ${isSold ? "sold" : ""}`;
  el.textContent = label;
  el.title = `${id} | ${type === "vip" ? "VIP" : type === "couple" ? "Couple" : "Thường"} | ${fmtCurrency(price)}`;
  el.dataset.id = id;
  el.dataset.type = type;
  el.dataset.price = price;
  if (!isSold) el.addEventListener("click", () => toggleSeat(el));
  return el;
}

function toggleSeat(el) {
  const id = el.dataset.id;
  const type = el.dataset.type;
  const price = +el.dataset.price;
  const idx = bs.selectedSeats.findIndex((s) => s.id === id);

  if (idx > -1) {
    el.classList.remove("selected");
    bs.selectedSeats.splice(idx, 1);
    fetch(resolveRootPath() + "api/unlock-seat", {
      method: "POST", headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ showtimeId: bs.showtime.id, seatId: id })
    });
  } else {
    if (el.classList.contains("locked")) {
      showToast("Ghế này đang có người chọn!", "error");
      return;
    }
    if (bs.selectedSeats.length >= 8) {
      showToast("⚠️ Tối đa 8 ghế!", "error");
      return;
    }
    el.classList.add("selected");
    bs.selectedSeats.push({ id, type, price });
    fetch(resolveRootPath() + "api/lock-seat", {
      method: "POST", headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ showtimeId: bs.showtime.id, seatId: id })
    });
  }
  calcTotal();
  const btnGoToCombo = document.getElementById("btnGoToCombo");
  if (btnGoToCombo) btnGoToCombo.style.display = bs.selectedSeats.length > 0 ? "block" : "none";
}

function goToCombo() {
  if (bs.selectedSeats.length === 0) {
    showToast("Vui lòng chọn ghế trước khi tiếp tục!", "error");
    return;
  }
  document.getElementById("comboSection").style.display = "block";
  document.getElementById("btnGoToCombo").style.display = "none";
  setStep(3);
  document.getElementById("comboSection").scrollIntoView({ behavior: "smooth", block: "start" });
}

function updateCombo(comboId, delta) {
  let qty = bs.comboItems[comboId] || 0;
  qty += delta;
  if (qty < 0) qty = 0;
  if (qty > 10) {
    showToast("⚠️ Chỉ được chọn tối đa 10 combo mỗi loại!", "error");
    return;
  }
  bs.comboItems[comboId] = qty;
  document.getElementById("combo" + comboId + "Qty").textContent = qty;
  
  // Calculate combo total
  const price1 = 79000;
  const price2 = 109000;
  bs.comboTotal = bs.comboItems[1] * price1 + bs.comboItems[2] * price2;
  
  calcTotal();
}

function goToPayment() {
  const paySection = document.getElementById("paymentSection");
  if (paySection) {
    paySection.style.display = "block";
    paySection.scrollIntoView({ behavior: "smooth", block: "start" });
  }
  showMemberInfo();
  calcTotal();
  setStep(4);
}

// Hàm fetch danh sách ghế đang bị người khác khóa
async function refreshLockedSeats(showtimeId) {
  try {
    const res = await fetch(resolveRootPath() + `api/locked-seats?showtimeId=${showtimeId}`);
    const lockedSeats = await res.json();
    document.querySelectorAll(".seat").forEach(el => {
      const id = el.dataset.id;
      // Bỏ qua ghế đã bán và ghế mình đang chọn
      if (el.classList.contains("sold") || el.classList.contains("selected")) return;
      
      if (lockedSeats.includes(id)) {
        el.classList.add("locked");
        el.style.opacity = "0.5";
        el.title = "Đang có người chọn";
      } else {
        el.classList.remove("locked");
        el.style.opacity = "1";
        const type = el.dataset.type;
        const price = el.dataset.price;
        el.title = `${id} | ${type === "vip" ? "VIP" : type === "couple" ? "Couple" : "Thường"} | ${fmtCurrency(price)}`;
      }
    });
  } catch (e) {
    console.error("Lỗi lấy ghế khóa:", e);
  }
}

// ── TOTAL CALC ──
function calcTotal() {
  const user = AuthService.currentUser();
  bs.subtotal = bs.selectedSeats.reduce((s, x) => s + x.price, 0);
  
  const totalBeforeDiscount = bs.subtotal + (bs.comboTotal || 0);

  // Member discount (tương đương Customer::tinhGiamGia())
  const tier = normalizeTier(user?.tier);
  const discRate = MEMBER_DISCOUNT[tier] || 0;
  bs.memberDisc = Math.floor(totalBeforeDiscount * discRate);

  // Points discount
  const pointsUsed = Math.min(
    +(document.getElementById("pointsInput")?.value || 0),
    user?.points || 0,
  );
  bs.pointsUsed = pointsUsed;
  bs.pointsDisc = pointsUsed * VND_PER_POINT;

  bs.total = Math.max(0, totalBeforeDiscount - bs.memberDisc - bs.pointsDisc);

  // Card fee
  if (bs.payMethod === "card") bs.total = Math.round(bs.total * 1.015);

  updateSummaryUI();
}

function updateSummaryUI() {
  const el = (id) => document.getElementById(id);
  const tags = el("seatTagsEl");
  if (!tags) return;
  if (bs.selectedSeats.length === 0) {
    tags.innerHTML =
      '<span style="font-size:.78rem;color:var(--text-400)">Chưa chọn ghế</span>';
    const totalEl = el("totalVal");
    if (totalEl) totalEl.textContent = "0đ";
    const btnConfirm = el("btnConfirm");
    if (btnConfirm) btnConfirm.disabled = true;
    const subtotalRow = el("subtotalRow");
    if (subtotalRow) subtotalRow.style.display = "none";
    const memberDiscRow = el("memberDiscRow");
    if (memberDiscRow) memberDiscRow.style.display = "none";
    const pointsDiscRow = el("pointsDiscRow");
    if (pointsDiscRow) pointsDiscRow.style.display = "none";
    return;
  }

  tags.innerHTML = bs.selectedSeats
    .map((s) => `<span class="seat-tag ${s.type}">${s.id}</span>`)
    .join("");

  const subtotalRow = el("subtotalRow");
  if (subtotalRow) subtotalRow.style.display = "flex";
  const subtotalVal = el("subtotalVal");
  if (subtotalVal) subtotalVal.textContent = fmtCurrency(bs.subtotal);

  const comboRow = el("comboRow");
  if (comboRow) comboRow.style.display = bs.comboTotal > 0 ? "flex" : "none";
  const comboVal = el("comboVal");
  if (comboVal) comboVal.textContent = "+" + fmtCurrency(bs.comboTotal);

  const memberDiscRow = el("memberDiscRow");
  if (memberDiscRow)
    memberDiscRow.style.display = bs.memberDisc > 0 ? "flex" : "none";
  const memberDiscVal = el("memberDiscVal");
  if (memberDiscVal)
    memberDiscVal.textContent = "-" + fmtCurrency(bs.memberDisc);

  const user = AuthService.currentUser();
  const tier = normalizeTier(user?.tier);
  const memberDiscLbl = el("memberDiscLbl");
  if (memberDiscLbl)
    memberDiscLbl.textContent = `Giảm hạng ${tier} (${Math.round((MEMBER_DISCOUNT[tier] || 0) * 100)}%)`;

  const pointsDiscRow = el("pointsDiscRow");
  if (pointsDiscRow)
    pointsDiscRow.style.display = bs.pointsDisc > 0 ? "flex" : "none";
  const pointsDiscVal = el("pointsDiscVal");
  if (pointsDiscVal)
    pointsDiscVal.textContent = "-" + fmtCurrency(bs.pointsDisc);

  const totalVal = el("totalVal");
  if (totalVal) totalVal.textContent = fmtCurrency(bs.total);
  const btnConfirm = el("btnConfirm");
  if (btnConfirm) btnConfirm.disabled = false;
}

function updateSummaryHeader(suat, movie) {
  const room = RoomService.getById(suat.roomId);
  const sumShowtime = document.getElementById("sumShowtime");
  if (sumShowtime) sumShowtime.textContent = `${suat.date} lúc ${suat.time}`;
  const sumMovie = document.getElementById("sumMovie");
  if (sumMovie) sumMovie.textContent = movie?.title || "—";
  const sumDate = document.getElementById("sumDate");
  if (sumDate) sumDate.textContent = fmtDate(suat.date);
  const sumTime = document.getElementById("sumTime");
  if (sumTime)
    sumTime.textContent = suat.time + (suat.special ? " ⭐ Premier" : "");
  const sumRoom = document.getElementById("sumRoom");
  if (sumRoom) sumRoom.textContent = room?.name || suat.roomId;
}

// ── MEMBER / POINTS ──
function showMemberInfo() {
  const user = AuthService.currentUser();
  const box = document.getElementById("memberInfoBox");
  const psec = document.getElementById("pointsSection");
  if (!user || user.role === "admin" || !box || !psec) return;

  const tier = user.tier || "Bronze";
  const disc = (MEMBER_DISCOUNT[tier] || 0) * 100;
  box.style.display = "block";
  box.innerHTML = `${getTierBadge(tier)} Hạng <strong style="color:${getTierColor(tier)}">${tier}</strong> – giảm <strong>${disc}%</strong> mọi vé | Điểm: <strong style="color:var(--gold)">${user.points} ⭐</strong>`;

  if (user.points >= 1) {
    psec.style.display = "block";
    const availPoints = document.getElementById("availPoints");
    if (availPoints) availPoints.textContent = user.points;
    const availPointsVnd = document.getElementById("availPointsVnd");
    if (availPointsVnd)
      availPointsVnd.textContent = fmtCurrency(user.points * VND_PER_POINT);
  }
}

function applyMaxPoints() {
  const user = AuthService.currentUser();
  if (!user) return;
  const maxPossible = Math.floor((bs.subtotal + (bs.comboTotal || 0)) / VND_PER_POINT);
  const pointsInput = document.getElementById("pointsInput");
  if (pointsInput) pointsInput.value = Math.min(user.points, maxPossible);
  calcTotal();
}

// ── PAYMENT ──
let paymentInterval = null;

// ── XỬ LÝ CHỌN PHƯƠNG THỨC THANH TOÁN & SINH QR ──
function selectPay(method) {
  document
    .querySelectorAll(".pay-opt")
    .forEach((el) => el.classList.remove("active"));
  document.getElementById("pay_" + method)?.classList.add("active");
  bs.payMethod = method;
  calcTotal();

  const qrBox = document.getElementById("qrPaymentBox");
  const qrImg = document.getElementById("qrPaymentImg");

  if (paymentInterval) clearInterval(paymentInterval);

  if (
    ["vnpay", "momo", "zalopay"].includes(method) &&
    bs.selectedSeats.length > 0
  ) {
    const description =
      `CMAX_${bs.showtime.id}_${bs.selectedSeats.map((s) => s.id).join("")}`.replace(
        /\s+/g,
        "",
      );
    let qrUrl = "";

    const orderId = "HD" + Math.floor(100000 + Math.random() * 900000);
    bs.currentOrderId = orderId;

    // Sử dụng chuẩn VietQR để tương thích 100% với MoMo, ZaloPay, VNPay, và các app Ngân hàng
    const bankBin = "970422"; // MBBank (Mã BIN chuẩn VietQR)
    const bankAccount = "0123456789"; // Số tài khoản demo
    const accountName = "CINEMAX VN";
    const amount = bs.total;
    const msg = `CINEMAX ${orderId}`;
    
    qrUrl = `https://img.vietqr.io/image/${bankBin}-${bankAccount}-compact2.png?amount=${amount}&addInfo=${encodeURIComponent(msg)}&accountName=${encodeURIComponent(accountName)}`;

    if (qrImg) qrImg.src = qrUrl;
    if (qrBox) qrBox.style.display = "flex"; 

    startCheckingPaymentStatus();
  } else {
    if (qrBox) qrBox.style.display = "none";
  }
}

// ── HÀM VÒNG LẶP KIỂM TRA ĐĂNG KÝ NGẦM (POLLING) ──
function startCheckingPaymentStatus() {
  if (paymentInterval) clearInterval(paymentInterval);

  paymentInterval = setInterval(async () => {
    if (!bs.currentOrderId) return;

    try {
      const response = await fetch(
        resolveRootPath() + `api/check-payment?orderId=${bs.currentOrderId}`,
      );
      const data = await response.json();

      if (data.paid === true || data.status === "paid") {
        clearInterval(paymentInterval);

        const qrBox = document.getElementById("qrPaymentBox");
        if (qrBox) {
          qrBox.style.display = "none"; 
        }

        await confirmBooking();
      }
    } catch (err) {
      console.error("Lỗi khi check trạng thái thanh toán ngầm:", err);
    }
  }, 3000); 
}

function closeQRModal() {
  if (paymentInterval) clearInterval(paymentInterval);
  const qrBox = document.getElementById("qrPaymentBox");
  if (qrBox) qrBox.style.display = "none";
}

window.forcePaymentSuccess = async function() {
  if (paymentInterval) clearInterval(paymentInterval);
  const qrBox = document.getElementById("qrPaymentBox");
  if (qrBox) qrBox.style.display = "none";
  await confirmBooking();
};

window.onbeforeunload = function () {
  if (paymentInterval) clearInterval(paymentInterval);
};

// ── CONFIRM BOOKING ──
async function confirmBooking() {
  const user = AuthService.currentUser();
  if (!user) {
    showToast("⚠️ Vui lòng đăng nhập!", "error");
    openLoginModal();
    return;
  }
  if (!bs.showtime || bs.selectedSeats.length === 0) {
    showToast("⚠️ Chưa chọn ghế!", "error");
    return;
  }

  const btn = document.getElementById("btnConfirm");
  if (btn) {
    btn.textContent = "⏳ Đang kết nối server...";
    btn.disabled = true;
  }

  const PAY_MAP = { vnpay: "0", momo: "1", zalopay: "2" };

  const payload = {
    userId: String(user.id),
    showtimeId: String(bs.showtime.id),
    seats: bs.selectedSeats.map((s) => s.id),
    payMethod: String(PAY_MAP[bs.payMethod] || 0),
    pointsUsed: String(bs.pointsUsed || 0),
    comboPrice: bs.comboTotal || 0,
  };

  try {
    const response = await fetch(resolveRootPath() + "api/book", {
      method: "POST",
      headers: {
        "Content-Type": "application/json; charset=utf-8",
      },
      body: JSON.stringify(payload),
    });

    if (!response.ok) {
      throw new Error(`Server báo lỗi HTTP: ${response.status}`);
    }

    const result = await response.json();

    if (result.ok === false || result.status === "error") {
      showToast(
        "❌ " + (result.msg || result.message || "Đặt vé thất bại!"),
        "error",
      );
      if (btn) {
        btn.textContent = "🎟️ Xác nhận đặt vé";
        btn.disabled = false;
      }
      return;
    }

    const finalBooking = {
      bookingCode:
        result.bookingCode ||
        result.ticketCode ||
        result.id ||
        "CMAX" + Math.floor(100000 + Math.random() * 900000),
      total: result.totalPrice || result.total || bs.total,
      payMethod: bs.payMethod,
      seats: bs.selectedSeats,
      showtimeId: bs.showtime.id,
      memberDiscount: bs.memberDisc,
      pointsUsed: bs.pointsUsed,
      comboPrice: bs.comboTotal || 0,
    };

    const updatedUser = result.updatedUser || {
      ...user,
      points: user.points - bs.pointsUsed + Math.floor(bs.total / 100000),
    };

    if (typeof AuthService.updateCurrentUser === "function") {
      AuthService.updateCurrentUser(updatedUser);
    } else {
      localStorage.setItem("currentUser", JSON.stringify(updatedUser));
    }

    showSuccessModal(finalBooking, updatedUser);

    if (btn) {
      btn.textContent = "🎟️ Xác nhận đặt vé";
      btn.disabled = false;
    }
    setStep(5);
    showToast("🎉 Đặt vé thành công!", "success");
  } catch (err) {
    console.error("Lỗi đặt vé:", err);
    showToast("❌ Hệ thống không phản hồi hoặc C++ Error!", "error");
    if (btn) {
      btn.textContent = "🎟️ Xác nhận đặt vé";
      btn.disabled = false;
    }
  }
}

function showSuccessModal(booking, updatedUser) {
  const codeDisplay = document.getElementById("bkCodeDisplay");
  if (codeDisplay) codeDisplay.textContent = booking.bookingCode;
  const earnedPoints = Math.floor(booking.total / POINTS_PER_VND);
  const ticketQrImg = document.getElementById("ticketQrImg");
  if (ticketQrImg) {
    // Sử dụng API tạo mã QR miễn phí và gán trực tiếp vào thuộc tính src của thẻ ảnh
    ticketQrImg.src = `https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=${encodeURIComponent(booking.bookingCode)}`;
  }
  const summary = document.getElementById("successSummary");
  const movieObj = MovieService.getById(
    booking.showtimeId
      ? ShowtimeService.getById(booking.showtimeId)?.movieId
      : "",
  );
  const movieTitle =
    movieObj?.title || booking.movieTitle || bs.movie?.title || "Phim";
  const suatObj = ShowtimeService.getById(booking.showtimeId);
  const roomObj = suatObj ? RoomService.getById(suatObj.roomId) : null;
  const roomName = roomObj?.name || booking.roomName || "Phòng";
  const showTime = suatObj?.time || booking.showTime || "—";
  const showDate = suatObj ? fmtDate(suatObj.date) : booking.showDate || "—";

  if (summary) {
    summary.innerHTML = `
      <p>🎬 <strong>${movieTitle}</strong></p>
      <p>🕐 <strong>${showTime}</strong> · ${showDate} · ${roomName}</p>
      <p>💺 Ghế: <strong>${booking.seats.map((s) => s.id).join(", ")}</strong></p>
      <p>💳 Thanh toán: <strong>${booking.payMethod.toUpperCase()}</strong></p>
      <p>💰 Tổng tiền: <strong style="color:var(--primary)">${fmtCurrency(booking.total)}</strong></p>
      ${booking.memberDiscount > 0 ? `<p>🏆 Đã giảm hạng thành viên: <strong style="color:#1db954">-${fmtCurrency(booking.memberDiscount)}</strong></p>` : ""}
      ${earnedPoints > 0 ? `<p>⭐ Bạn vừa nhận <strong style="color:var(--gold)">${earnedPoints} điểm</strong> tích lũy!</p>` : ""}
      ${updatedUser ? `<p>📊 Tổng điểm hiện tại: <strong style="color:var(--gold)">${updatedUser.points} điểm</strong></p>` : ""}`;
  }

  const modal = document.getElementById("successModal");
  if (modal) modal.classList.add("open");
  refreshNavAuth();
}

// ── STEPS ──
function setStep(n) {
  [1, 2, 3, 4, 5].forEach((i) => {
    const s = document.getElementById("step" + i);
    const l = document.getElementById("line" + i);
    if (s) s.className = "step" + (i < n ? " done" : i === n ? " active" : "");
    if (l) l.className = "step-line" + (i < n ? " done" : "");
  });
}

// Expose globals so HTML can call them
Object.assign(window, {
  selectShowtime,
  toggleSeat,
  calcTotal,
  applyMaxPoints,
  selectPay,
  confirmBooking,
  showSuccessModal,
  closeQRModal,
  goToCombo,
  updateCombo,
  goToPayment,
});
