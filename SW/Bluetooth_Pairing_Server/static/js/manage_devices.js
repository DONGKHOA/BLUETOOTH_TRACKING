let selectedSSID = "";

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
  if (!selectedSSID || !password) {
    alert("Please select a WiFi and enter a password.");
    return;
  }

  try {
    const res = await fetch("/send_wifi", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ ssid: selectedSSID, password })
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