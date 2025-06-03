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

    function updateStatus(cardId, textId, status, statusColors) {
        $(`#${textId}`).text(status.charAt(0).toUpperCase() + status.slice(1)); 
        let colorClass = statusColors[status] || "red";
        $(`#${cardId}`).removeClass("red green").addClass(colorClass);
    }

    function loadCurrentConfig() {
        $.get("/get_current_config", function (response) {
            if (response.success) {
                let config = response.config;

                // Update AWS IoT Endpoint
                $("#awsEndpointDisplay").text(config.aws_iot_endpoint);

                // Update Facility ID
                $("#facilityIDDisplay").text(config.facility_id);

                // Ensure facilities exist before updating table
                if (config.facilities && config.facilities.length > 0) {
                    let tableRows = config.facilities.map(facility => `
                        <tr>
                            <td>${facility.id}</td>
                            <td>${facility.name}</td>
                            <td>${facility.address || "N/A"}</td>
                            <td>${facility.timezone}</td>
                            <td>${facility.tcp_server_name}</td>
                        </tr>
                    `).join("");

                    $("#facilityTableBody").html(tableRows);  // Insert rows into table
                    $("#facilityCount").text(config.count);   // Update total count
                } else {
                    $("#facilityTableBody").html("<tr><td colspan='5'>No facilities found.</td></tr>");
                    $("#facilityCount").text("0");
                }
            } else {
                $("#configDisplay").html("<p style='color: red;'>Error fetching configuration.</p>");
            }
        }).fail(function () {
            $("#configDisplay").html("<p style='color: red;'>Failed to load server configuration.</p>");
        });
    }

    function checkServerSession() {
        $.get("/check_session", function(response) {
            if (!response.valid) {
                alert(response.message);
    
                // Disable buttons and prevent further actions
                $("button").prop("disabled", true).css("opacity", "0.5");
    
                // Redirect user to login page after 1 second
                setTimeout(function() {
                    window.location.href = "/login";
                }, 1000);
            }
        }).fail(function () {
            console.error("Error checking session.");
        });
    }

    // Call the functions when the page loads
    fetchServerStatus();
    loadCurrentConfig();
    setInterval(fetchServerStatus, 2000);
    setInterval(checkServerSession, 5000);
});
