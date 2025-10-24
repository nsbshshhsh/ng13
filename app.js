// ================== MQTT CONFIG ==================
const clientId = "web_dashboard_";
const options = {
  clientId,
  username: "han13",
  password: "Han@1328",
  clean: true,
  reconnectPeriod: 1000,
  connectTimeout: 30 * 1000,
  protocolVersion: 4,
  rejectUnauthorized: false,
};

const client = mqtt.connect(
  "wss://944c54f3d9844fb5acd3ddb7387320b4.s1.eu.hivemq.cloud:8884/mqtt",
  options
);

// ================== ELEMENTS ==================
const ppmValue = document.getElementById("ppm-value");
const statusBadge = document.getElementById("status-badge");
const buzzerStatus = document.getElementById("buzzer-status");
const ctx = document.getElementById("gasChart").getContext("2d");

// ================== CHART ==================
const gasData = [];
const timeLabels = [];
const warnThreshold = 2000;
const dangerThreshold = 2500;

const gasChart = new Chart(ctx, {
  type: "line",
  data: {
    labels: timeLabels,
    datasets: [
      {
        label: "Nồng độ gas (ppm)",
        data: gasData,
        borderColor: "#3b82f6",
        backgroundColor: "rgba(59,130,246,0.2)",
        tension: 0.3,
        fill: true,
        pointRadius: 2,
      },
      {
        label: "Ngưỡng cảnh báo",
        data: [],
        borderColor: "#facc15",
        borderDash: [6, 4],
        borderWidth: 2,
        pointRadius: 0,
      },
      {
        label: "Ngưỡng nguy hiểm",
        data: [],
        borderColor: "#ef4444",
        borderDash: [6, 4],
        borderWidth: 2,
        pointRadius: 0,
      },
    ],
  },
  options: {
    responsive: true,
    animation: false,
    scales: {
      x: {
        type: "time",
        time: {
          unit: "second",
          displayFormats: { second: "HH:mm:ss" },
        },
        ticks: {
          autoSkip: true,
          maxTicksLimit: 10,
        },
      },
      y: {
        beginAtZero: true,
        title: {
          display: true,
          text: "Nồng độ khí gas (ppm)",
        },
      },
    },
    plugins: {
      legend: { position: "top" },
    },
  },
});

// ================== MQTT EVENTS ==================
client.on("connect", () => {
  console.log("✅ MQTT Connected to HiveMQ Cloud");
  client.subscribe([
    "gas_sensor/ppm",
    "gas_sensor/status",
    "gas_sensor/buzzer",
  ]);
});

client.on("error", (err) => {
  console.error("❌ MQTT Error:", err);
});

client.on("reconnect", () => {
  console.warn("🔁 Reconnecting to MQTT...");
});

client.on("close", () => {
  console.warn("🚫 MQTT Disconnected");
});

client.on("message", (topic, message) => {
  const msg = message.toString();

  if (topic === "gas_sensor/ppm") {
    updateDashboard(parseFloat(msg));
  } else if (topic === "gas_sensor/status") {
    statusBadge.innerText =
      msg === "Warning"
        ? "Cảnh báo"
        : msg === "Danger"
        ? "Nguy hiểm"
        : "An toàn";
    statusBadge.className =
      msg === "Warning"
        ? "status warning"
        : msg === "Danger"
        ? "status danger"
        : "status safe";
  } else if (topic === "gas_sensor/buzzer") {
    buzzerStatus.className =
      msg === "ON" ? "buzzer-status on" : "buzzer-status off";
    buzzerStatus.innerHTML = `
      <strong>Trạng thái:</strong> Còi đang <span>${
        msg === "ON" ? "BẬT" : "TẮT"
      }</span>
    `;
  }
});

// ================== CẬP NHẬT DASHBOARD ==================
function updateDashboard(ppm) {
  ppmValue.innerText = ppm.toFixed(1) + " ppm";

  // Màu sắc và trạng thái
  if (ppm < warnThreshold) {
    statusBadge.innerText = "An toàn";
    statusBadge.className = "status safe";
    ppmValue.style.color = "#22c55e";
  } else if (ppm < dangerThreshold) {
    statusBadge.innerText = "Cảnh báo";
    statusBadge.className = "status warning";
    ppmValue.style.color = "#eab308";
  } else {
    statusBadge.innerText = "Nguy hiểm";
    statusBadge.className = "status danger";
    ppmValue.style.color = "#dc2626";
  }

  // Cập nhật biểu đồ
  const now = new Date();
  gasData.push({ x: now, y: ppm });
  gasChart.data.datasets[1].data.push({ x: now, y: warnThreshold });
  gasChart.data.datasets[2].data.push({ x: now, y: dangerThreshold });

  // Giữ lại 10 điểm gần nhất
  if (gasData.length > 10) {
    gasData.shift();
    gasChart.data.datasets[1].data.shift();
    gasChart.data.datasets[2].data.shift();
  }

  // Tự động điều chỉnh trục Y
  const maxValue = Math.max(...gasData.map((d) => d.y), dangerThreshold) * 1.1;
  gasChart.options.scales.y.max = Math.ceil(maxValue / 100) * 100;

  gasChart.update("none");
}

// ================== BẬT/TẮT CÒI ==================
function toggleBuzzer(on) {
  const msg = on ? "ON" : "OFF";
  client.publish("gas_sensor/buzzer", msg);

  buzzerStatus.className = on ? "buzzer-status on" : "buzzer-status off";
  buzzerStatus.innerHTML = `
    <strong>Trạng thái:</strong> Còi đang <span>${
      on ? "BẬT" : "TẮT"
    }</span><br/>
    <small>${
      on
        ? "Còi đang hoạt động, cảnh báo!"
        : "Còi tắt, không có cảnh báo âm thanh"
    }</small>
  `;
}

// ================== GÁN SỰ KIỆN NÚT ==================
document
  .getElementById("btnOn")
  .addEventListener("click", () => toggleBuzzer(true));
document
  .getElementById("btnOff")
  .addEventListener("click", () => toggleBuzzer(false));
