$(document).ready(function() {
    function fetchServerStatus() {
        $.get("/check_status_server", function (response) {
            // Update the TCP Server status UI
            updateStatus("tcpServerStatus", "tcpStatusText", response.tcp_status, {
                "running": "green",
                "stopping": "red",
                "timeout": "red"
            });

            // Update the AWS Server status UI
            updateStatus("awsServerStatus", "awsStatusText", response.aws_status, {
                "connected": "green",
                "disconnected": "red",
                "timeout": "red"
            });
        }).fail(function () {
            $("#tcpStatusText, #awsStatusText").text("Error");
        });
    }

    function fetchDeviceManagement() {
        $.get("/get_device_management", function (response) {
            var tableBody = $("#deviceManageTable tbody");
            tableBody.empty();
            
            response.devices.forEach(function(device) {
                var row = `<tr><td>${device.id}</td><td>${device.last_message}</td></tr>`;
                tableBody.append(row);
            });
        }).fail(function () {
            console.log("Failed to fetch device management data");
        });
    }

    function updateStatus(cardId, textId, status, statusColors) {
        $(`#${textId}`).text(status.charAt(0).toUpperCase() + status.slice(1)); 
        let colorClass = statusColors[status] || "red";
        $(`#${cardId}`).removeClass("red green").addClass(colorClass);
    }

    // Call the functions when the page loads
    fetchServerStatus();
    fetchDeviceManagement();
});
