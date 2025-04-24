$(document).ready(function () {

    fetchFacilityList()

    // Start Server Button Click
    $("#startServer").click(function () {
        $.post("/start_server", function (response) {
            $("#serverMessage").html("<p style='color: green;'>" + response.message + "</p>");
        }).fail(function () {
            $("#serverMessage").html("<p style='color: red;'>Error starting the server.</p>");
        });
    });

    // Stop Server Button Click
    $("#stopServer").click(function () {
        $.get("/stop_server", function (response) {
            $("#serverMessage").html("<p style='color: green;'>" + response.message + "</p>");
        }).fail(function () {
            $("#serverMessage").html("<p style='color: red;'>Error stopping the server.</p>");
        });
    });

    // Handle Authentication Form Submit
    $("#AuthenticateForm").submit(function (event) {
        event.preventDefault(); // Prevent page reload

        $("#AuthenticateButton").prop("disabled", true).text("Authenticating..."); // Disable button

        $.post("/authenticate", $(this).serialize(), function (response) {
            if (response.success) {
                // Login successful → Redirect
                $("#authMessage").css("color", "green").text("Login successful! Redirecting...");
                setTimeout(() => {
                    window.location.href = response.redirect;
                }, 1000); // Redirect after 1 second
            } else {
                // Wrong login → Show error message instead of redirecting
                $("#authMessage").css("color", "red").text("Wrong username or password.");
                $("#AuthenticateButton").prop("disabled", false).text("Authenticate"); // Re-enable button
            }
        }).fail(function (xhr) {
            // Server error → Show error instead of redirecting
            $("#authMessage").css("color", "red").text("Server error! Please try again.");
            $("#AuthenticateButton").prop("disabled", false).text("Authenticate");
        });
    });
    // Handle Facility ID Submission
    $("#facilityForm").submit(function (event) {
        event.preventDefault(); // Prevent page refresh

        var selectedId = $("#facility_id").val(); // Get selected facility ID

        $.post("/select_facility_id", $(this).serialize(), function (response) {
            $("#facilityMessage").html("<p style='color: green;'>Facility ID = \"" + selectedId + "\" updated successfully!</p>");
        }).fail(function () {
            $("#facilityMessage").html("<p style='color: red;'>Error updating Facility ID.</p>");
        });
    });
})

// Fetch Facility List from API and Update UI
function fetchFacilityList() {
    console.log("Fetching facility list..."); // Debugging

    $.get("/facility_list", function(response) {
        console.log("Facility List Response:", response); // Debugging

        if (response.success) {
            var tableBody = $("#facilityTable tbody");
            tableBody.empty(); // Clear existing rows

            response.data.forEach(function(item) {
                var row = `
                    <tr>
                        <td>${item.id}</td>
                        <td>${item.name}</td>
                        <td>${item.address}</td>
                        <td>${item.timezone}</td>
                        <td>${item.tcp_server_name}</td>
                    </tr>
                `;
                tableBody.append(row);
            });

            $("#facilityCount").text(response.count);
        } else {
            $("#facilityMessage").html("<p style='color: red;'>No facilities found.</p>");
        }
    }).fail(function() {
        $("#facilityMessage").html("<p style='color: red;'>Error fetching facility list.</p>");
    });
}