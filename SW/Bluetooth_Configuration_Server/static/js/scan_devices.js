let selectedMAC = null;

async function scanDevices() {
  const res = await fetch("/scan");
  const devices = await res.json();

  const list = document.getElementById("deviceList");
  list.innerHTML = "";

  devices.forEach(dev => {
    const li = document.createElement("li");

    const info = document.createElement("div");
    info.className = "device-info";

    const name = document.createElement("div");
    name.className = "device-name";
    name.textContent = dev.name;

    const mac = document.createElement("div");
    mac.className = "device-mac";
    mac.textContent = dev.address;

    info.appendChild(name);
    info.appendChild(mac);

    const rssi = document.createElement("div");
    rssi.textContent = dev.rssi ? `${dev.rssi} dBm` : "";

    li.appendChild(info);
    li.appendChild(rssi);

    // Click to connect
    li.style.cursor = "pointer";
    li.onclick = async () => {
      selectedMAC = dev.address;

      const connectRes = await fetch(`/connect/${encodeURIComponent(dev.address)}`);
      const connectData = await connectRes.json();

      if (connectData.connected) {
        alert(`Connected to ${dev.name} (${dev.address})`);

        sessionStorage.setItem("connectedMac", dev.address);
        sessionStorage.setItem("connectedName", dev.name);
        
        // Optional: navigate to a device page
        window.location.href = `/manage_device/${encodeURIComponent(dev.address)}`;
      } else {
        alert("Failed to connect: " + (connectData.error || "Unknown error"));
      }
    };

    list.appendChild(li);
  });
}
