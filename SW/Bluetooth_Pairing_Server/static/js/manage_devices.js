let selectedSSID = "";

document.getElementById("mac").textContent = sessionStorage.getItem("connectedMac");
document.getElementById("name").textContent = sessionStorage.getItem("connectedName");

function showTab(id) {
  document.querySelectorAll('.tab-content').forEach(tab => tab.classList.remove('active'));
  document.getElementById(id).classList.add('active');

  if (id !== 'network') {
    closeWifiModal();
  }
}

async function openWifi() {
    document.getElementById("wifiModal").style.display = "flex";
    document.getElementById("wifiPasswordSection").style.display = "none";
    document.getElementById("wifiPassword").value = "";
    selectedSSID = "";
  
    const list = document.getElementById("wifiList");
    list.innerHTML = "Scanning for networks...";
  
    try {
      const res = await fetch("/scan_wifi");
      let data = await res.json();
  
      // 🔥 Remove duplicates using Set
      const uniqueSSIDs = [...new Set(data.filter(ssid => ssid && ssid.trim() !== ""))];
  
      list.innerHTML = "";
      uniqueSSIDs.forEach(ssid => {
        const li = document.createElement("li");
        li.textContent = ssid;
        li.style.cursor = "pointer";
        li.className = "wifi-item";
        li.onclick = () => {
          selectedSSID = ssid;
          highlightSelectedWifi(li);
          document.getElementById("wifiPasswordSection").style.display = "block";
        };
        list.appendChild(li);
      });
  
    } catch (err) {
      list.textContent = "Error scanning WiFi: " + err.message;
    }
  }  

function highlightSelectedWifi(selectedElement) {
    document.querySelectorAll("#wifiList li").forEach(el => {
        el.classList.remove("selected-wifi");
    });
    selectedElement.classList.add("selected-wifi");
    }

async function sendWifi() {
  const password = document.getElementById("wifiPassword").value;
  const mac = sessionStorage.getItem("connectedMac");
  if (!selectedSSID || !password || !mac) {
    alert("Please select a WiFi, enter a password, and ensure MAC is available.");
    return;
  }

  try {
    const res = await fetch("/send_wifi", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ ssid: selectedSSID, password, mac })
    });

    const data = await res.json();
    if (data.sent) {
      alert("WiFi credentials sent to ESP32!");
      document.getElementById("wifiModal").style.display = "none";

      // Update current SSID display if needed
      const ssidDisplay = document.querySelector("#network .info-row span");
      if (ssidDisplay) ssidDisplay.textContent = selectedSSID;

    } else {
      alert("Failed to send: " + data.error);
    }
  } catch (err) {
    alert("Error sending WiFi: " + err.message);
  }
}

function closeWifiModal() {
  document.getElementById("wifiModal").style.display = "none";
  document.getElementById("wifiPasswordSection").style.display = "none";
  document.getElementById("wifiPassword").value = "";
  selectedSSID = "";
}

function logout() {
  fetch("/disconnect")
    .then(res => res.json())
    .then(data => {
      console.log("Disconnected from ESP32:", data);
      // Clear session data (if you're storing MAC/name)
      sessionStorage.clear();
      // Redirect to home/login page
      window.location.href = "/";
    })
    .catch(err => {
      console.error("Failed to disconnect:", err);
      window.location.href = "/";
    });
}

document.addEventListener("DOMContentLoaded", async () => {
  const mac = sessionStorage.getItem("connectedMac");
  const name = sessionStorage.getItem("connectedName");

  if (mac) document.getElementById("mac").textContent = mac;
  if (name) document.getElementById("name").textContent = name;

  try {
    const res = await fetch(`/get_wifi_config/${encodeURIComponent(mac)}`);
    const data = await res.json();

    // Set current SSID to display in Network tab
    const ssidSpan = document.getElementById("currentSSID");
    if (ssidSpan && data.ssid) {
      ssidSpan.textContent = data.ssid;
      selectedSSID = data.ssid; // preload for modal reuse
    }

    // Optional: preload password into modal field
    const modalPassInput = document.getElementById("wifiPassword");
    if (modalPassInput && data.password) {
      modalPassInput.value = data.password;
    }

  } catch (err) {
    console.error("Failed to load previous WiFi config:", err);
  }
});

