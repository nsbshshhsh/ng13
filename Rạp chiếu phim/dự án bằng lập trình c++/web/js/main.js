// ============================================================
// main.js - JavaScript chính cho CineMax Web Interface
// Quản lý: dữ liệu phim, auth, lịch chiếu, UI interactions
// ============================================================

// ===== DỮ LIỆU GIẢ LẬP (tương ứng với C++ data) =====
const CINEMA_DATA = {
  movies: [
    {
      id: "MV001",
      title: "Avengers: Secret Wars",
      titleOrig: "Avengers: Secret Wars",
      genre: "Hành động",
      duration: 180,
      rating: 9.0,
      votes: 12500,
      ageLimit: 13,
      director: "Russo Brothers",
      cast: "Robert Downey Jr., Chris Evans",
      description:
        "Trận chiến cuối cùng của các Avengers chống lại Doctor Doom. Một bộ phim siêu anh hùng hoành tráng nhất từ trước đến nay!",
      poster: "images/mv001.jpg",
      releaseDate: "2024-05-01",
      status: "hot",
      trailer: "https://youtube.com",
    },
    {
      id: "MV002",
      title: "Gia Đình Là Số 1",
      titleOrig: "Family Is First",
      genre: "Tình cảm/Hài",
      duration: 120,
      rating: 8.5,
      votes: 8200,
      ageLimit: 0,
      director: "Victor Vu",
      cast: "Trấn Thành, Hari Won",
      description:
        "Câu chuyện đầy cảm xúc về gia đình và tình yêu thương. Bộ phim sẽ khiến bạn cười và khóc xen kẽ.",
      poster: "images/mv002.jpg",
      releaseDate: "2024-04-20",
      status: "new",
      trailer: "https://youtube.com",
    },
    {
      id: "MV003",
      title: "Vong Bóng Tối",
      titleOrig: "Shadow of Darkness",
      genre: "Kinh dị",
      duration: 105,
      rating: 7.8,
      votes: 5400,
      ageLimit: 18,
      director: "James Wan",
      cast: "Patrick Wilson",
      description:
        "Một gia đình chuyển đến ngôi nhà ma ám và phải đối mặt với thế lực bóng tối không thể giải thích.",
      poster: "images/mv003.jpg",
      releaseDate: "2024-05-10",
      status: "hot",
      trailer: "https://youtube.com",
    },
    {
      id: "MV004",
      title: "Doraemon: Nobita Phiêu Lưu",
      titleOrig: "Doraemon: New Adventure",
      genre: "Hoạt hình",
      duration: 95,
      rating: 8.8,
      votes: 15000,
      ageLimit: 0,
      director: "Yoshihiro Shimizu",
      cast: "Megumi Oohara, Yumi Kakazu",
      description:
        "Doraemon và Nobita trong cuộc phiêu lưu mới kỳ diệu, khám phá vùng đất bí ẩn chưa ai biết đến.",
      poster: "images/mv004.jpg",
      releaseDate: "2024-05-05",
      status: "new",
      trailer: "https://youtube.com",
    },
    {
      id: "MV005",
      title: "Lướt Sóng Đại Dương",
      titleOrig: "Ocean Surf",
      genre: "Hành động",
      duration: 130,
      rating: 7.5,
      votes: 3200,
      ageLimit: 16,
      director: "Michael Bay",
      cast: "Chris Hemsworth",
      description:
        "Một nhóm lính cứu hộ đối mặt với cơn bão thế kỷ tại vùng biển Thái Bình Dương.",
      poster: "images/mv005.jpg",
      releaseDate: "2024-05-15",
      status: "",
      trailer: "https://youtube.com",
    },
    {
      id: "MV006",
      title: "Tình Yêu Trong Mưa",
      titleOrig: "Love in the Rain",
      genre: "Tình cảm",
      duration: 110,
      rating: 8.2,
      votes: 9800,
      ageLimit: 13,
      director: "Park Chan-wook",
      cast: "Jun Ji-hyun, Lee Min-ho",
      description:
        "Câu chuyện tình yêu lãng mạn giữa hai người trẻ tìm thấy nhau trong những ngày mưa.",
      poster: "images/mv006.jpg",
      releaseDate: "2024-04-28",
      status: "",
      trailer: "https://youtube.com",
    },
  ],

  showtimes: [
    {
      id: "ST001",
      movieId: "MV001",
      roomId: "P03",
      roomName: "IMAX Hall",
      time: "09:30",
      date: "2026-06-10",
      price: 150000,
      available: 45,
      type: "IMAX",
      lang: "Phụ đề Việt",
    },
    {
      id: "ST002",
      movieId: "MV001",
      roomId: "P02",
      roomName: "Phòng 2 (3D)",
      time: "14:00",
      date: "2026-06-10",
      price: 100000,
      available: 58,
      type: "3D",
      lang: "Thuyết minh",
    },
    {
      id: "ST003",
      movieId: "MV001",
      roomId: "P03",
      roomName: "IMAX Hall",
      time: "19:30",
      date: "2026-06-10",
      price: 150000,
      available: 12,
      type: "IMAX Premier",
      lang: "Phụ đề Việt",
      special: true,
    },

    {
      id: "ST004",
      movieId: "MV002",
      roomId: "P01",
      roomName: "Phòng 1 (2D)",
      time: "10:00",
      date: "2026-06-10",
      price: 80000,
      available: 72,
      type: "2D",
      lang: "Thuyết minh",
    },
    {
      id: "ST005",
      movieId: "MV002",
      roomId: "P01",
      roomName: "Phòng 1 (2D)",
      time: "15:30",
      date: "2026-06-10",
      price: 80000,
      available: 60,
      type: "2D",
      lang: "Thuyết minh",
    },
    {
      id: "ST006",
      movieId: "MV003",
      roomId: "P02",
      roomName: "Phòng 2 (3D)",
      time: "21:00",
      date: "2026-06-10",
      price: 100000,
      available: 88,
      type: "3D",
      lang: "Phụ đề Việt",
    },
    {
      id: "ST007",
      movieId: "MV004",
      roomId: "P01",
      roomName: "Phòng 1 (2D)",
      time: "13:00",
      date: "2026-06-10",
      price: 80000,
      available: 95,
      type: "2D",
      lang: "Thuyết minh",
    },
    {
      id: "ST008",
      movieId: "MV005",
      roomId: "P04",
      roomName: "Premium Recliner",
      time: "16:00",
      date: "2026-06-10",
      price: 200000,
      available: 30,
      type: "Premium",
      lang: "Phụ đề Việt",
    },
    {
      id: "ST009",
      movieId: "MV006",
      roomId: "P01",
      roomName: "Phòng 1 (2D)",
      time: "18:00",
      date: "2026-06-10",
      price: 80000,
      available: 55,
      type: "2D",
      lang: "Thuyết minh",
    },
    // Ngày mai
    {
      id: "ST010",
      movieId: "MV001",
      roomId: "P03",
      roomName: "IMAX Hall",
      time: "10:00",
      date: "2026-06-11",
      price: 150000,
      available: 120,
      type: "IMAX",
      lang: "Phụ đề Việt",
    },
    {
      id: "ST011",
      movieId: "MV002",
      roomId: "P01",
      roomName: "Phòng 1 (2D)",
      time: "14:00",
      date: "2026-06-11",
      price: 80000,
      available: 96,
      type: "2D",
      lang: "Thuyết minh",
    },
  ],

  users: [
    {
      id: 101,
      email: "an.tran@gmail.com",
      password: "123456",
      name: "Trần Văn An",
      phone: "0912345678",
      role: "customer",
      points: 50,
      tier: "SILVER 🥈",
    },
    {
      id: 102,
      email: "binh.le@gmail.com",
      password: "123456",
      name: "Lê Thị Bình",
      phone: "0923456789",
      role: "customer",
      points: 0,
      tier: "BRONZE 🥉",
    },
    {
      id: 1,
      email: "admin@cinema.vn",
      password: "admin123",
      name: "Nguyễn Quản Trị",
      phone: "0901234567",
      role: "admin",
      points: 0,
      tier: "STAFF",
    },
  ],
};

// ===== STATE MANAGEMENT =====
let appState = {
  currentUser: null,
  selectedDate: null,
  bookings: JSON.parse(localStorage.getItem("cinemax_bookings") || "[]"),
  users: JSON.parse(
    localStorage.getItem("cinemax_users") || JSON.stringify(CINEMA_DATA.users),
  ),
};

// ===== KHỞI TẠO =====
document.addEventListener("DOMContentLoaded", () => {
  initNavbar();
  loadCurrentUser();
  if (document.getElementById("moviesGrid")) renderMovieGrid();
  if (document.getElementById("scheduleGrid")) renderSchedule();
  if (document.getElementById("dateTabs")) renderDateTabs();
  animateStats();
  createParticles();
});

// ===== NAVBAR =====
function initNavbar() {
  const navbar = document.getElementById("navbar");
  if (!navbar) return;
  window.addEventListener("scroll", () => {
    navbar.classList.toggle("scrolled", window.scrollY > 50);
  });
}

function toggleMenu() {
  const navLinks = document.getElementById("navLinks");
  navLinks.classList.toggle("open");
}

function scrollToSchedule() {
  document.getElementById("lichChieu")?.scrollIntoView({ behavior: "smooth" });
}

// ===== PARTICLES (Hero Section) =====
function createParticles() {
  const container = document.getElementById("particles");
  if (!container) return;
  for (let i = 0; i < 30; i++) {
    const p = document.createElement("div");
    p.className = "particle";
    p.style.cssText = `
      left: ${Math.random() * 100}%;
      --duration: ${5 + Math.random() * 10}s;
      --delay: ${Math.random() * -15}s;
      width: ${2 + Math.random() * 4}px;
      height: ${2 + Math.random() * 4}px;
      opacity: ${0.3 + Math.random() * 0.7};
    `;
    container.appendChild(p);
  }
}

// ===== COUNTER ANIMATION =====
function animateStats() {
  const counters = document.querySelectorAll(".stat-number[data-count]");
  const observer = new IntersectionObserver(
    (entries) => {
      entries.forEach((entry) => {
        if (!entry.isIntersecting) return;
        const el = entry.target;
        const target = parseInt(el.dataset.count);
        const duration = 1500;
        const start = performance.now();
        const animate = (time) => {
          const progress = Math.min((time - start) / duration, 1);
          const ease = 1 - Math.pow(1 - progress, 3);
          el.textContent = Math.floor(ease * target).toLocaleString("vi-VN");
          if (progress < 1) requestAnimationFrame(animate);
        };
        requestAnimationFrame(animate);
        observer.unobserve(el);
      });
    },
    { threshold: 0.5 },
  );
  counters.forEach((c) => observer.observe(c));
}

// ===== RENDER MOVIE GRID =====
function renderMovieGrid(filter = "all") {
  const grid = document.getElementById("moviesGrid");
  if (!grid) return;
  grid.innerHTML = "";

  const movies =
    filter === "all"
      ? CINEMA_DATA.movies
      : CINEMA_DATA.movies.filter((m) => m.genre.includes(filter));

  movies.forEach((m, idx) => {
    const card = createMovieCard(m, idx);
    grid.appendChild(card);
  });
}

function createMovieCard(movie, idx = 0) {
  const card = document.createElement("div");
  card.className = "movie-card";
  card.style.animationDelay = `${idx * 0.08}s`;

  const stars = "⭐".repeat(Math.floor(movie.rating / 2));
  const ageBadge = movie.ageLimit === 0 ? "P" : `T${movie.ageLimit}`;

  card.innerHTML = `
    <div class="movie-poster">
      <div class="movie-poster-placeholder">${getPosterHTML(movie.poster)}</div>
      ${movie.status ? `<div class="movie-badge ${movie.status}">${movie.status === "hot" ? "🔥 HOT" : "✨ MỚI"}</div>` : ""}
      <div class="movie-rating">⭐ ${movie.rating.toFixed(1)}</div>
      <div class="movie-overlay">
        <button class="btn-book-now" onclick="bookMovie('${movie.id}')">🎟️ Đặt vé ngay</button>
      </div>
    </div>
    <div class="movie-info">
      <div class="movie-title" title="${movie.title}">${movie.title}</div>
      <div class="movie-meta">
        <span class="movie-genre">${movie.genre}</span>
        <span class="movie-age">${ageBadge}</span>
        <span>⏱ ${formatDuration(movie.duration)}</span>
      </div>
    </div>
  `;

  card.addEventListener("click", (e) => {
    if (!e.target.closest(".btn-book-now")) {
      window.location.href = `booking.html?movie=${movie.id}`;
    }
  });
  return card;
}

// ===== RENDER DATE TABS =====
function renderDateTabs() {
  const container = document.getElementById("dateTabs");
  if (!container) return;

  const today = new Date();
  const days = ["CN", "T2", "T3", "T4", "T5", "T6", "T7"];
  const months = [
    "Th1",
    "Th2",
    "Th3",
    "Th4",
    "Th5",
    "Th6",
    "Th7",
    "Th8",
    "Th9",
    "Th10",
    "Th11",
    "Th12",
  ];

  for (let i = 0; i < 7; i++) {
    const d = new Date(today);
    d.setDate(today.getDate() + i);
    const dateStr = d.toISOString().split("T")[0];

    const tab = document.createElement("button");
    tab.className = "date-tab" + (i === 0 ? " active" : "");
    tab.innerHTML = `
      <span class="day-name">${i === 0 ? "Hôm nay" : days[d.getDay()]}</span>
      <span class="day-num">${d.getDate()}</span>
      <span class="day-name">${months[d.getMonth()]}</span>
    `;
    tab.onclick = () => {
      document
        .querySelectorAll(".date-tab")
        .forEach((t) => t.classList.remove("active"));
      tab.classList.add("active");
      appState.selectedDate = dateStr;
      renderSchedule(dateStr);
    };
    container.appendChild(tab);
  }

  // Set default date
  appState.selectedDate = today.toISOString().split("T")[0];
  renderSchedule(appState.selectedDate);
}

// ===== RENDER SCHEDULE =====
function renderSchedule(dateFilter) {
  const grid = document.getElementById("scheduleGrid");
  if (!grid) return;
  grid.innerHTML = "";

  // Lọc suất chiếu theo ngày (demo dùng ngày 2026-06-10)
  let suats = CINEMA_DATA.showtimes;

  // Group theo phim
  const grouped = {};
  suats.forEach((s) => {
    if (!grouped[s.movieId]) grouped[s.movieId] = [];
    grouped[s.movieId].push(s);
  });

  const movieIds = Object.keys(grouped);
  if (movieIds.length === 0) {
    grid.innerHTML =
      '<p style="text-align:center;color:var(--text-400)">Không có suất chiếu nào.</p>';
    return;
  }

  movieIds.forEach((mid) => {
    const movie = CINEMA_DATA.movies.find((m) => m.id === mid);
    if (!movie) return;

    const row = document.createElement("div");
    row.className = "schedule-movie-row";

    const timesHTML = grouped[mid]
      .map(
        (s) => `
      <button class="time-btn ${s.available < 20 ? "low-seats" : ""}"
              onclick="selectShowtime('${s.id}')"
              title="${s.roomName} | ${s.lang}">
        <span>${s.time}</span>
        <span class="time-type">${s.type}</span>
        <span class="time-seats">${s.available} ghế còn</span>
      </button>
    `,
      )
      .join("");

    row.innerHTML = `
      <div class="schedule-movie-header">
        <div class="schedule-poster">${getPosterHTML(movie.poster)}</div>
        <div class="schedule-movie-info">
          <div class="schedule-movie-name">${movie.title}</div>
          <div class="schedule-movie-meta">
            <span>⏱ ${formatDuration(movie.duration)}</span>
            <span>${movie.genre}</span>
            <span>⭐ ${movie.rating}</span>
          </div>
        </div>
      </div>
      <div class="schedule-times">${timesHTML}</div>
    `;
    grid.appendChild(row);
  });
}

function selectShowtime(showtimeId) {
  const suat = CINEMA_DATA.showtimes.find((s) => s.id === showtimeId);
  if (!suat) return;
  window.location.href = `booking.html?showtime=${showtimeId}`;
}

function bookMovie(movieId) {
  window.location.href = `booking.html?movie=${movieId}`;
}

// ===== AUTH FUNCTIONS =====
function loadCurrentUser() {
  const saved = localStorage.getItem("cinemax_user");
  if (saved) {
    appState.currentUser = JSON.parse(saved);
    updateNavAuth(true);
  } else {
    updateNavAuth(false);
  }
}

function updateNavAuth(isLoggedIn) {
  const loginBtn = document.getElementById("loginBtn");
  const logoutBtn = document.getElementById("logoutBtn");
  const userInfo = document.getElementById("userInfo");
  const myTicketsBtn = document.getElementById("myTicketsBtn");

  if (isLoggedIn && appState.currentUser) {
    loginBtn && (loginBtn.style.display = "none");
    logoutBtn && (logoutBtn.style.display = "inline-flex");
    userInfo && (userInfo.style.display = "flex");
    myTicketsBtn && (myTicketsBtn.style.display = "inline-flex");
    if (userInfo) {
      userInfo.innerHTML = `👤 ${appState.currentUser.name.split(" ").pop()}`;
    }
  } else {
    loginBtn && (loginBtn.style.display = "inline-flex");
    logoutBtn && (logoutBtn.style.display = "none");
    userInfo && (userInfo.style.display = "none");
    myTicketsBtn && (myTicketsBtn.style.display = "none");
  }
}

function openLoginModal() {
  document.getElementById("loginModal").classList.add("open");
  document.body.style.overflow = "hidden";
}

function closeLoginModal() {
  document.getElementById("loginModal").classList.remove("open");
  document.body.style.overflow = "";
  document.getElementById("loginError").textContent = "";
  document.getElementById("regError") &&
    (document.getElementById("regError").textContent = "");
}

// Đóng modal khi click ra ngoài
document.addEventListener("DOMContentLoaded", () => {
  const overlay = document.getElementById("loginModal");
  overlay?.addEventListener("click", (e) => {
    if (e.target === overlay) closeLoginModal();
  });
});

function switchTab(tab) {
  const loginForm = document.getElementById("loginForm");
  const registerForm = document.getElementById("registerForm");
  const loginTab = document.getElementById("loginTab");
  const registerTab = document.getElementById("registerTab");

  if (tab === "login") {
    loginForm.style.display = "block";
    registerForm.style.display = "none";
    loginTab.classList.add("active");
    registerTab.classList.remove("active");
  } else {
    loginForm.style.display = "none";
    registerForm.style.display = "block";
    loginTab.classList.remove("active");
    registerTab.classList.add("active");
  }
}

function fillDemo(email, password) {
  document.getElementById("loginEmail").value = email;
  document.getElementById("loginPassword").value = password;
}

function togglePassword(id) {
  const input = document.getElementById(id);
  input.type = input.type === "password" ? "text" : "password";
}

function handleLogin() {
  const email = document.getElementById("loginEmail").value.trim();
  const password = document.getElementById("loginPassword").value;
  const errEl = document.getElementById("loginError");

  if (!email || !password) {
    errEl.textContent = "⚠️ Vui lòng nhập đầy đủ thông tin!";
    return;
  }

  const users = appState.users;
  const user = users.find((u) => u.email === email && u.password === password);

  if (user) {
    appState.currentUser = { ...user };
    localStorage.setItem("cinemax_user", JSON.stringify(appState.currentUser));
    closeLoginModal();
    updateNavAuth(true);
    showToast(`✅ Chào mừng, ${user.name.split(" ").pop()}!`, "success");

    // Redirect admin
    if (user.role === "admin") {
      setTimeout(() => (window.location.href = "admin/dashboard.html"), 800);
    }
  } else {
    errEl.textContent = "❌ Email hoặc mật khẩu không đúng!";
  }
}

function handleRegister() {
  const name = document.getElementById("regName").value.trim();
  const email = document.getElementById("regEmail").value.trim();
  const phone = document.getElementById("regPhone").value.trim();
  const password = document.getElementById("regPassword").value;
  const errEl = document.getElementById("regError");

  if (!name || !email || !phone || !password) {
    errEl.textContent = "⚠️ Vui lòng nhập đầy đủ thông tin!";
    return;
  }
  if (password.length < 6) {
    errEl.textContent = "❌ Mật khẩu phải có ít nhất 6 ký tự!";
    return;
  }
  if (appState.users.find((u) => u.email === email)) {
    errEl.textContent = "❌ Email này đã được đăng ký!";
    return;
  }

  const newUser = {
    id: Date.now(),
    email,
    password,
    name,
    phone,
    role: "customer",
    points: 0,
    tier: "BRONZE 🥉",
  };
  appState.users.push(newUser);
  localStorage.setItem("cinemax_users", JSON.stringify(appState.users));

  showToast("✅ Đăng ký thành công! Vui lòng đăng nhập.", "success");
  switchTab("login");
  document.getElementById("loginEmail").value = email;
}

function logout() {
  appState.currentUser = null;
  localStorage.removeItem("cinemax_user");
  updateNavAuth(false);
  showToast("👋 Đã đăng xuất thành công!", "success");
}

// ===== TOAST NOTIFICATION =====
function showToast(message, type = "info") {
  let toast = document.getElementById("globalToast");
  if (!toast) {
    toast = document.createElement("div");
    toast.id = "globalToast";
    toast.className = "toast";
    document.body.appendChild(toast);
  }
  toast.textContent = message;
  toast.className = `toast ${type}`;
  requestAnimationFrame(() => {
    toast.classList.add("show");
    setTimeout(() => toast.classList.remove("show"), 3500);
  });
}

// ===== UTILITY FUNCTIONS =====
function formatDuration(minutes) {
  const h = Math.floor(minutes / 60);
  const m = minutes % 60;
  return h > 0 ? `${h}g${m > 0 ? m + "p" : ""}` : `${m}p`;
}

function formatCurrency(amount) {
  return new Intl.NumberFormat("vi-VN", {
    style: "currency",
    currency: "VND",
  }).format(amount);
}

function formatDate(dateStr) {
  const d = new Date(dateStr);
  return d.toLocaleDateString("vi-VN", {
    weekday: "long",
    year: "numeric",
    month: "long",
    day: "numeric",
  });
}

// Expose to global
window.openLoginModal = openLoginModal;
window.closeLoginModal = closeLoginModal;
window.switchTab = switchTab;
window.fillDemo = fillDemo;
window.togglePassword = togglePassword;
window.handleLogin = handleLogin;
window.handleRegister = handleRegister;
window.logout = logout;
window.bookMovie = bookMovie;
window.selectShowtime = selectShowtime;
window.scrollToSchedule = scrollToSchedule;
window.toggleMenu = toggleMenu;
window.CINEMA_DATA = CINEMA_DATA;
window.appState = appState;
window.showToast = showToast;
window.formatCurrency = formatCurrency;
window.formatDuration = formatDuration;
